#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

#define PPP_FLAG 0x7E
#define PPP_ESC  0x7D
#define PPP_ADDR 0xFF
#define PPP_CTRL 0x03


struct Node {
    char url[50], ip[20], mac[20];
    int port;
    struct Node *next;
};

struct Node *table[SIZE] = {NULL};

char srcURL[50] = "Default Source";
char srcIP[20] = "192.168.1.10";
char srcMAC[20] = "11:22:33:44:55:66";
int srcPort = 51309;

int hash(char url[]) {
    int sum = 0;
    for(int i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}

void insert(char url[], char ip[], char mac[], int port) {
    int idx = hash(url);
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->url, url);
    strcpy(newNode->ip, ip);
    strcpy(newNode->mac, mac);
    newNode->port = port;
    newNode->next = table[idx];
    table[idx] = newNode;
}

struct Node* search(char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void delete(char url[]) {
    int idx = hash(url);
    struct Node *tmp = table[idx];
    struct Node *prev = NULL;

    while (tmp != NULL && strcmp(tmp->url, url) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL) return;

    if (prev == NULL) {
        table[idx] = tmp->next;
    } else {
        prev->next = tmp->next;
    }
    free(tmp);
}

void URLTable() {
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-19s %-5s\n", "URL", "IP", "MAC", "PORT");
    for(int i = 0; i < SIZE; i++) {
        struct Node *tmp = table[i];
        while(tmp) {
            printf("%-20s %-18s %-19s %-5d\n", tmp->url, tmp->ip, tmp->mac, tmp->port);
            tmp = tmp->next;
        }
    }
    printf("===========================================\n");
}

void preload() {
    insert("www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25);
    insert("www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443);
    insert("www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 80);
    insert("www.google.com", "142.250.190.47", "AA:BB:CC:DD:EE:04", 443);
}

void printByte(unsigned char n) {
    for (int i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}

void printPort(int port) {
    for (int i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}

void printIP(char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}

void printMAC(char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByte(x[i]);
    }
}

unsigned short calculateChecksum(char data[])
{
    unsigned int sum = 0;
    unsigned short word;
    int len = strlen(data);

    for(int i = 0; i < len; i += 16)
    {
        word = 0;

        for(int j = 0; j < 16; j++)
        {
            word <<= 1;

            if(i + j < len && data[i+j] == '1')
                word |= 1;
        }

        sum += word;

        while(sum > 0xFFFF)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (unsigned short)(~sum);
}

void checksumToBinary(unsigned short checksum,char bits[])
{
    for(int i=15;i>=0;i--)
    {
        if(checksum & (1<<i))
            bits[15-i]='1';
        else
            bits[15-i]='0';
    }

    bits[16]='\0';
}

int verifyChecksum(char data[])
{
    unsigned int sum = 0;
    unsigned short word;
    int len = strlen(data);

    for(int i=0;i<len;i+=16)
    {
        word=0;

        for(int j=0;j<16;j++)
        {
            word <<=1;

            if(i+j<len && data[i+j]=='1')
                word |=1;
        }

        sum += word;

        while(sum>0xFFFF)
            sum=(sum&0xFFFF)+(sum>>16);
    }

    sum=~sum;

    return ((sum&0xFFFF)==0);
}



void showLayers(char msg[], int len, struct Node *dest) {
    printf("\nMessage :\n");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);

    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    printIP(srcIP); printf(" "); printIP(dest->ip); printf(" ");
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    printMAC(srcMAC); printf(" "); printMAC(dest->mac); printf(" ");
    printIP(srcIP); printf(" "); printIP(dest->ip); printf(" ");
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64 + 96);
}

void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    printf("====Frame Contents======\n");
    for(int i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int packetNo = (i / 2) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPort(srcPort); printf("\n");
        printf("Destination Port : "); printPort(dest->port); printf("\n\n");
        printf("Source IP      : "); printIP(srcIP); printf("\n");
        printf("Destination IP : "); printIP(dest->ip); printf("\n");
        printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
        printf("Destination MAC: "); printMAC(dest->mac); printf("\n");

        printf("Frame Data     : ");
        for(int j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) { printByte(msg[cur]); printf(" "); }
            else { printByte(0); printf(" "); }
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}

void flipBit(char bits[], int pos)
{
    int len = strlen(bits);

    if(pos < 1 || pos > len)
    {
        printf("Invalid bit position!\n");
        return;
    }

    char current = bits[pos - 1];
    char newBit;

    printf("Current bit at position %d = %c\n", pos, current);
    printf("Enter new bit (0 or 1): ");
    scanf(" %c", &newBit);

    if(newBit != '0' && newBit != '1')
    {
        printf("Invalid bit entered!\n");
        return;
    }

    if(newBit == current)
    {
        printf("No change made.\n");
    }
    else
    {
        bits[pos - 1] = newBit;
        printf("Bit changed successfully.\n");
    }
}

void sender_PPP(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_ppp.txt", "w");
    if (!fp) return;
    char frameBits[10000] = "";
    
    
    char temp[9];
    char checksumBits[17];
    unsigned short checksum;

    printf("\n========= SENDER INTERMEDIATE PROCESSING (PPP) =========\n");

    // 1. Start Flag
    fprintf(fp, "%02X ", PPP_FLAG);
    printf("-> Appending Start Flag      : "); printByte(PPP_FLAG); printf("\n");

    // 2. Address & Control Fields
    fprintf(fp, "%02X %02X ", PPP_ADDR, PPP_CTRL);
    printf("-> Appending Address Block   : "); printByte(PPP_ADDR); printf("\n");
    printf("-> Appending Control Block   : "); printByte(PPP_CTRL); printf("\n");

    /* Store Address in frameBits */
    for(int b = 7; b >= 0; b--)
    {
        temp[7 - b] = ((PPP_ADDR >> b) & 1) + '0';
    }
    temp[8] = '\0';
    strcat(frameBits, temp);

    /* Store Control in frameBits */
    for(int b = 7; b >= 0; b--)
    {
        temp[7 - b] = ((PPP_CTRL >> b) & 1) + '0';  
    }
    temp[8] = '\0';
    strcat(frameBits, temp);

    // 3. Protocol field (0x00 0x21)
    fprintf(fp, "00 21 ");
    printf("-> Appending Protocol Ident. : "); printByte(0x00); printf(" "); printByte(0x21); printf("\n");

    /* Display Original Payload */
    printf("\nOriginal Payload (Binary):\n");
    for (int i = 0; i < len; i++) {
        printByte(data[i]); printf(" ");
    }
    printf("\n");

    /* Store Protocol High Byte (0x00) */
    for(int b = 7; b >= 0; b--)
    {
        temp[7 - b] = ((0x00 >> b) & 1) + '0';
    }
    temp[8] = '\0';
    strcat(frameBits, temp);

    /* Store Protocol Low Byte (0x21) */
    for(int b = 7; b >= 0; b--)
    {
        temp[7 - b] = ((0x21 >> b) & 1) + '0';
    }
    temp[8] = '\0';
    strcat(frameBits, temp);

    /* Display Complete Stuffed Frame Header info */
    printf("\n Stuffed Frame\n");
    printByte(PPP_FLAG); printf(" "); printByte(PPP_ADDR); printf(" ");
    printByte(PPP_CTRL); printf(" "); printByte(0x00); printf(" "); printByte(0x21); printf(" ");

    // 4. Stuffed Payload Data
    printf("\n-> Processing and Byte-Stuffing Payload Stream:\n   Final Frame Sequence: [ ");

    for (int i = 0; i < len; i++) {
        if (data[i] == PPP_FLAG || data[i] == PPP_ESC) {
            fprintf(fp, "%02X ", PPP_ESC);
            fprintf(fp, "%02X ", data[i]);

            printByte(PPP_ESC); printf(" "); printByte(data[i]); printf(" ");
        } else {
            fprintf(fp, "%02X ", data[i]);

            printByte(data[i]); printf(" ");
        }
        /* Add Payload Byte to frameBits */
        for(int b = 7; b >= 0; b--)
        {
            temp[7 - b] = ((data[i] >> b) & 1) + '0';
        }
        temp[8] = '\0';
        strcat(frameBits, temp);

    }
    checksum = calculateChecksum(frameBits);
    checksumToBinary(checksum, checksumBits);
    printf("]\n");

    
    /* Store checksum in file */
    fprintf(fp, "%02X %02X ",
        (checksum >> 8) & 0xFF,
        checksum & 0xFF);

   
    printByte((checksum >> 8) & 0xFF);
    printf(" ");
    printByte(checksum & 0xFF);
    printf(" ");
    printByte(PPP_FLAG);
    printf("\n");

    // 6. End Flag
    fprintf(fp, "%02X\n", PPP_FLAG);
    printf("-> Appending End Flag        : "); printByte(PPP_FLAG); printf("\n");
    printf("========================================================\n");

    fclose(fp);
    printf("[Sender] Full structured PPP Frame saved to 'transmitted_ppp.txt'\n");
}

void modifyTransmissionFile(int payloadLength)
{
    FILE *fp = fopen("transmitted_ppp.txt","r");

    if(fp==NULL)
    {
        printf("Cannot open transmission file!\n");
        return;
    }

    unsigned int data[1000];
    int count=0;

    while(fscanf(fp,"%x",&data[count])==1)
        count++;

    fclose(fp);

    int choice;

    printf("\nDo you want to introduce an error? (1-Yes / 0-No): ");
    scanf("%d",&choice);

    if(choice==0)
        return;

    int bytePos, bitPos;
    char newBit;

    int payloadStart = 6;
    int payloadEnd = payloadStart + payloadLength - 1;  // Last payload byte (before checksum and end flag)

    printf("Enter payload byte position (%d-%d): ", payloadStart, payloadEnd);
    scanf("%d", &bytePos);

    if(bytePos < payloadStart || bytePos > payloadEnd)
    {
        printf("Please modify only payload bytes!\n");
        return;
    }

    printf("Enter bit position inside the byte (1-8): ");
    scanf("%d", &bitPos);
    if(bitPos < 1 || bitPos > 8)
    {
        printf("Invalid bit position!\n");
        return;
    }

    if(bytePos<1 || bytePos>count)
    {
        printf("Invalid Position\n");
        return;
    }

    printf("Current Byte : ");
    printByte(data[bytePos-1]);
    printf("\n");

    int currentBit = (data[bytePos-1] >> (8-bitPos)) & 1;

    printf("Current Bit = %d\n", currentBit);

    printf("Enter new bit (0 or 1): ");
    scanf(" %c", &newBit);

    if(newBit!='0' && newBit!='1')
    {
        printf("Invalid Bit!\n");
        return;
    }

    if((newBit-'0') == currentBit)
    {
        printf("No change made.\n");
    }
    else
    {
        if(newBit=='1')
            data[bytePos-1] |= (1 << (8-bitPos));
        else
            data[bytePos-1] &= ~(1 << (8-bitPos));

        printf("Bit changed successfully.\n");
    }

    printf("Modified Byte : ");
    printByte(data[bytePos-1]);
    printf("\n");

    fp=fopen("transmitted_ppp.txt","w");

    for(int i=0;i<count;i++)
        fprintf(fp,"%02X ",data[i]);
    fprintf(fp,"\n");

    fclose(fp);
}

void receiver_PPP()
{
    FILE *fp = fopen("transmitted_ppp.txt","r");

    if(fp==NULL)
    {
        printf("Error reading transmission file!\n");
        return;
    }

    unsigned int frame[1000];
    int count = 0;

    while(fscanf(fp,"%x",&frame[count]) == 1)
        count++;

    fclose(fp);

    char frameBits[10000] = "";
    char temp[9];

    printf("\n========= RECEIVER SIDE (PPP BYTE DESTUFFING) =========\n");

    printf("Start Flag : ");
    printByte(frame[0]);
    printf("\n");

    printf("Address    : ");
    printByte(frame[1]);
    printf("\n");

    printf("Control    : ");
    printByte(frame[2]);
    printf("\n");

    /* Address */
    for(int b=7;b>=0;b--)
        temp[7-b]=((frame[1]>>b)&1)+'0';
    temp[8]='\0';
    strcat(frameBits,temp);

    /* Control */
    for(int b=7;b>=0;b--)
        temp[7-b]=((frame[2]>>b)&1)+'0';
    temp[8]='\0';
    strcat(frameBits,temp);

    /* Protocol */
    for(int i=3;i<=4;i++)
    {
        for(int b=7;b>=0;b--)
            temp[7-b]=((frame[i]>>b)&1)+'0';
        temp[8]='\0';
        strcat(frameBits,temp);
    }

    /* Payload = from byte 5 to count-4 */
    for(int i=5;i<=count-4;i++)
    {
        for(int b=7;b>=0;b--)
            temp[7-b]=((frame[i]>>b)&1)+'0';
        temp[8]='\0';
        strcat(frameBits,temp);
    }

    unsigned short receivedChecksum =
        (frame[count-3] << 8) | frame[count-2];

    unsigned int sum = 0;
    unsigned short word;

    /* Calculate sum of all received data words */
    for(int i = 0; i < strlen(frameBits); i += 16)
    {
        word = 0;

        for(int j = 0; j < 16; j++)
        {
            word <<= 1;

            if(i + j < strlen(frameBits) && frameBits[i+j] == '1')
                word |= 1;
        }

        sum += word;

        while(sum > 0xFFFF)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }

    printf("\nReceived Checksum : ");
    printByte((receivedChecksum >> 8) & 0xFF);
    printf(" ");
    printByte(receivedChecksum & 0xFF);

    /* Add received checksum */
    sum += receivedChecksum;

    while(sum > 0xFFFF)
        sum = (sum & 0xFFFF) + (sum >> 16);

    printf("\nFinal Sum : ");
    printByte((sum >> 8) & 0xFF);
    printf(" ");
    printByte(sum & 0xFF);

    printf("\n\nVerification Result:\n");

    if((sum & 0xFFFF) == 0xFFFF)
    {
        printf("No Error Detected.\n");
    }
    else
    {
        printf("Error Detected!\n");
    }
}

int main()
{
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;

    preload();

    while(1)
    {
        URLTable();

        printf("\n========= MAIN MENU (PPP PROTOCOL RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run PPP\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &m);

        if(m == 1)
        {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back\n");
            scanf("%d", &cho);

            if(cho == 1)
            {
                char newUrl[50], newIp[20], newMac[20];
                int newPort;

                printf("Enter URL: ");
                scanf("%49s", newUrl);

                printf("Enter IP: ");
                scanf("%19s", newIp);

                printf("Enter MAC: ");
                scanf("%19s", newMac);

                printf("Enter Port: ");
                scanf("%d", &newPort);

                insert(newUrl, newIp, newMac, newPort);
            }
            else if(cho == 2)
            {
                char delUrl[50];

                printf("Enter URL to delete: ");
                scanf("%49s", delUrl);

                delete(delUrl);
            }
        }
        else if(m == 2)
        {
            break;
        }
        else
        {
            return 0;
        }
    }

    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);

    struct Node *srcNode = search(url);

    if(srcNode)
    {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    }

    printf("Enter Destination URL: ");
    scanf("%99s", url);

    struct Node *dest = search(url);

    if(dest == NULL)
    {
        printf("Destination URL not found.\n");
        return 0;
    }

    printf("Enter File Name: ");
    scanf("%49s", fn);

    fp = fopen(fn, "r");

    if(fp == NULL)
    {
        printf("Unable to open file.\n");
        return 0;
    }

    while((ch = fgetc(fp)) != EOF && idx < 999)
        msg[idx++] = (char)ch;

    msg[idx] = '\0';

    printf("\nFile content = %s\n", msg);
    printf("Length = %d\n", strlen(msg));
    fclose(fp);

    int len = strlen(msg);
    int totalFrames = len / F_SZ + (len % F_SZ != 0);

    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);

    sender_PPP((unsigned char *)msg, len);

    modifyTransmissionFile(len);

    receiver_PPP();

    return 0;
}

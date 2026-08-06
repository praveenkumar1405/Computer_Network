#include <stdio.h>
#include <string.h>

void toBinary7(char ch, char out[])
{
    int value = ch;

    for (int i = 6; i >= 0; i--)
    {
        out[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }

    out[7] = '\0';
}

void addEvenParity(char data[], char byte[])
{
    int count = 0;

    for (int i = 0; i < 7; i++)
    {
        byte[i] = data[i];

        if (data[i] == '1')
            count++;
    }

    if (count % 2 == 0)
        byte[7] = '0';
    else
        byte[7] = '1';

    byte[8] = '\0';
}

void printByte(char out[])
{
    for (int i = 0; out[i] != '\0'; i++)
    {
        if (i > 0 && i % 8 == 0)
            printf(" ");

        printf("%c", out[i]);
    }

    printf("\n");
}

void parityCheck(char out[])
{
    int byteNo = 1;

    for (int i = 0; out[i] != '\0'; i += 8)
    {
        int count = 0;

        for (int j = 0; j < 8; j++)
        {
            if (out[i + j] == '1')
                count++;
        }

        printf("Byte %d : ", byteNo);

        if (count % 2 == 0)
            printf("Correct (Even Parity)\n");
        else
            printf("Wrong (Parity Error)\n");

        byteNo++;
    }
}

void change(char out[])
{
    int totalBytes = strlen(out) / 8;
    int targetByte, totalChanges, bitPos;
    char newBit;
    char choice;

    printf("\nDo you want to change any bit? (y/n): ");
    scanf(" %c", &choice);

    if (choice != 'y' && choice != 'Y')
        return;

    printf("Which byte do you want to change? (1-%d): ", totalBytes);
    scanf("%d", &targetByte);

    if (targetByte < 1 || targetByte > totalBytes)
    {
        printf("Invalid Byte!\n");
        return;
    }

    printf("How many bits do you want to change? ");
    scanf("%d", &totalChanges);

    for (int i = 0; i < totalChanges; i++)
    {
        printf("\nChange %d\n", i + 1);

        printf("Enter bit position (1-8): ");
        scanf("%d", &bitPos);

        if (bitPos < 1 || bitPos > 8)
        {
            printf("Invalid Position!\n");
            i--;
            continue;
        }

        int index = (targetByte - 1) * 8 + (bitPos - 1);

        if (out[index] == '0')
            out[index] = '1';
        else
            out[index] = '0';

        printf("Bit flipped successfully.\n");
    }

    printf("\nModified Data : ");
    printByte(out);
}

int main()
{
    char input[100];
    char result[800] = "";
    char seven[8];
    char byte[9];

    printf("Enter String : ");
    scanf("%s", input);

    for (int i = 0; input[i] != '\0'; i++)
    {
        toBinary7(input[i], seven);
        addEvenParity(seven, byte);
        strcat(result, byte);
    }

    printf("\nTransmitted Data:\n");
    printByte(result);

    printf("\nInitial Parity Check\n");
    parityCheck(result);

    change(result);

    printf("\nFinal Parity Check\n");
    parityCheck(result);

    return 0;
}

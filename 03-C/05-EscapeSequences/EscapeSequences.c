#include <stdio.h>

int main(void)
{
	// code
	printf("\n\n");
	printf("going to the next line using escape sequence \\n\n\n");
	printf("demonstrating\ta\thorizontal\ttab\tusing\tthe\tescape\tsequence\t\\t\n\n");
	printf("to print \"double quotes\", use the \\\" escape sequence\n\n");
	printf("to print \'single quotes\', use the \\\' escape sequence\n\n");
	printf("'backspace' turns to 'backspace\b' if you use the \\b escape sequence\n\n");

	printf("\r demonstrating the carriage return using escape sequence \\r\n");
	printf("demonstrating \r carriage return using escape sequence \\r\n");
	printf("demonstrating carriage \r return using escape sequence \\r\n\n");

	// 0x41 in hexadecimal notation = 65 in decimal notation = ASCII value of the character 'A'
	printf("when \\xhh = \\x41, you see \x41\n\n");

	// 0102 in octal notaion = 66 in decimal notation = ASCII value of the character 'B'
	printf("when \\ooo = \\102, you see \102\n\n");
	return(0);
}

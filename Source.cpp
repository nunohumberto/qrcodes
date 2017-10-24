#include "opencv2/opencv.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <bitset>
#include <cmath>
#define QRSIZE 25
using namespace std;
using namespace cv;


int checkDepth(vector<Vec4i> tree, int index) { /* Devolve a quantidade de filhos de um contorno */
	int depth = 0;
	int tempDepth = index;
	while (tree[tempDepth][2] != -1) {
		tempDepth = tree[tempDepth][2];
		depth += 1;
	}
	return depth;
}

double distancia(Point2f A, Point2f B) {
	return sqrt((B.x - A.x)*(B.x - A.x) + (B.y - A.y)*(B.y - A.y));
}

int** getDataMatrix(Mat QRcode, int side) { /* Converte a imagem processada para uma matriz de bits */
	int step = (QRcode.cols / side);
	int stride = step / 2;
	int** matrix = 0;
	matrix = new int*[side];

	for (int y = 0; y < side; y++) {

		matrix[y] = new int[side];

		for (int x = 0; x < side; x++) {
			matrix[y][x] = (QRcode.at<char>(y * step + stride, x * step + stride) == 0 ? 1 : 0);
		}

	}
	return matrix;


}

int ** unmask(int** matrix, int size) {
	int mask = -1;
	if (matrix[8][2] == 1 && matrix[8][3] == 1 && matrix[8][4] == 1) mask = 1;
	else if (matrix[8][2] == 1 && matrix[8][3] == 1 && matrix[8][4] == 0) mask = 2;
	else if (matrix[8][2] == 1 && matrix[8][3] == 0 && matrix[8][4] == 1) mask = 3;
	else if (matrix[8][2] == 1 && matrix[8][3] == 0 && matrix[8][4] == 0) mask = 4;
	else if (matrix[8][2] == 0 && matrix[8][3] == 1 && matrix[8][4] == 1) mask = 5;
	else if (matrix[8][2] == 0 && matrix[8][3] == 1 && matrix[8][4] == 0) mask = 6;
	else if (matrix[8][2] == 0 && matrix[8][3] == 0 && matrix[8][4] == 1) mask = 7;
	else if (matrix[8][2] == 0 && matrix[8][3] == 0 && matrix[8][4] == 0) mask = 8;

	for (int i = 0; i < size; i++) {
		if (i == 6) continue;
		for (int j = 0; j < size; j++) {
			if (j == 6) continue;
			if (i <= 8) {if (j <= 8 || j > (size - 9)) continue;}
			if (i >= (size - 9))  {if (j <= 8) continue;}
			if (i >= (size - 9) && i <= (size - 5)) {
				if (j >= (size - 9) && j <= (size - 5)) continue;
			}

			if (mask == 1) {
				if (j % 3 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 2) {
				if ((i+j) % 3 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 3) {
				if ((i+j) % 2 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 4) {
				if (i % 2 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 5) {
				if ( ((i * j) % 3 + i*j) % 2 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 6) {
				if ( ((i * j) % 3 + i+j) % 2 == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 7) {
				if ( (i/2 + j/3) % 2  == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}
			else if (mask == 8) {
				if ( (i*j)%2 + (i*j)%3  == 0) {
					if (matrix[i][j] == 0) matrix[i][j] = 1;
					else matrix[i][j] = 0;
				}
			}

		}
	}


	return matrix;
}

int getEncoding(int** matrix, int size) {
	int encoding = 0;
	encoding |= (matrix[size - 1][size - 1] << 3);
	encoding |= (matrix[size - 1][size - 2] << 2);
	encoding |= (matrix[size - 2][size - 1] << 1);
	encoding |= matrix[size - 2][size - 2];
	return encoding;
}

int getLength(int** matrix, int size, int encoding) {
	int length = 0;
	length |= (matrix[size - 3][size - 1] << 7);
	length |= (matrix[size - 3][size - 2] << 6);
	length |= (matrix[size - 4][size - 1] << 5);
	length |= (matrix[size - 4][size - 2] << 4);
	length |= (matrix[size - 5][size - 1] << 3);
	length |= (matrix[size - 5][size - 2] << 2);
	length |= (matrix[size - 6][size - 1] << 1);
	length |= (matrix[size - 6][size - 2]);
	if (encoding < 4) {
		length = length << 1;
		length |= (matrix[size - 7][size - 1]);
	}
	if (encoding < 2) {
		length = length << 1;
		length |= (matrix[size - 7][size - 2]);
	}
	return length;
}

bool checkAlignmentAndTiming(int** matrix, int size) {


	
	for (int i = 0; i < 7; i++) {
		/* Horizontal black position lines*/
		if (matrix[0][i] == 0) return false;
		if (matrix[0][(size - 1) - 6 + i] == 0) return false;
		if (matrix[6][i] == 0) return false;
		if (matrix[6][(size - 1) - 6 + i] == 0) return false;
		if (matrix[(size - 1) - 6][i] == 0) return false;
		if (matrix[size - 1][i] == 0) return false;
		/* Vertical black position lines*/
		if (matrix[i][0] == 0) return false;
		if (matrix[(size - 1) - 6 + i][0] == 0) return false;
		if (matrix[i][6] == 0) return false;
		if (matrix[(size - 1) - 6 + i][6] == 0) return false;
		if (matrix[i][(size - 1) - 6] == 0) return false;
		if (matrix[i][size - 1] == 0) return false;
	}
	

	
	for (int i = 0; i < 5; i++) {
		/* Horizontal white position lines*/
		if (matrix[1][i+1] == 1) return false;
		if (matrix[1][size - 6 + i] == 1) return false;
		if (matrix[5][i+1] == 1) return false;
		if (matrix[5][size - 6 + i] == 1) return false;
		if (matrix[size - 6][i+1] == 1) return false;
		if (matrix[size - 2][i+1] == 1) return false;
		/* Vertical white position lines*/
		if (matrix[i+1][1] == 1) return false;
		if (matrix[size - 6 + i][1] == 1) return false;
		if (matrix[i+1][5] == 1) return false;
		if (matrix[size - 6 + i][5] == 1) return false;
		if (matrix[i+1][(size) - 6] == 1) return false;
		if (matrix[i+1][size - 2] == 1) return false;
	}
	

	for (int i = 0; i < 3; i++) {
		/* Horizontal interior black position lines*/
		if (matrix[2][i+2] == 0) return false;
		if (matrix[2][(size + 1) - 6 + i] == 0) return false;
		if (matrix[4][i+2] == 0) return false;
		if (matrix[4][(size + 1) - 6 + i] == 0) return false;
		if (matrix[(size + 1) - 6][i+2] == 0) return false;
		if (matrix[size - 3][i+2] == 0) return false;
		/* Vertical interior black position lines*/
		if (matrix[i+2][2] == 0) return false;
		if (matrix[(size +1) - 6 + i][2] == 0) return false;
		if (matrix[i+2][4] == 0) return false;
		if (matrix[(size + 1) - 6 + i][4] == 0) return false;
		if (matrix[i+2][(size + 1) - 6] == 0) return false;
		if (matrix[i+2][size - 3] == 0) return false;
	}

	
	/* Final position check - centers */
	if (matrix[3][3] == 0 || matrix[3][size - 4] == 0 || matrix[size - 4][3] == 0) return false;

	/* Alignment marker check */

	for (int i = 0; i < 5; i++) {
		/* Horizontal black alignment lines*/
		if (matrix[size - 9][size - 9 + i] == 0) return false;
		if (matrix[size - 5][size - 9 + i] == 0) return false;
		/* Vertical black alignment lines */
		if (matrix[size - 9 + i][size - 9] == 0) return false;
		if (matrix[size - 9 + i][size - 5] == 0) return false;
	}
	
	for (int i = 0; i < 3; i++) {
		/* Horizontal white alignment lines*/
		if (matrix[size - 8][size - 8 + i] == 1) return false;
		if (matrix[size - 6][size - 8 + i] == 1) return false;
		/* Vertical white alignment lines */
		if (matrix[size - 8 + i][size - 8] == 1) return false;
		if (matrix[size - 8 + i][size - 6] == 1) return false;
	}
	

	/* Final position check - center */

	if (matrix[size - 7][size - 7] == 0) return false;


	/* Timing check */

	int last = 1;
	for (int i = 7; i < (size - 7); i++) {
		if (matrix[6][i] == last) return false;
		if (last == 1) last = 0;
		else last = 1;
	}
	last = 1;
	for (int i = 7; i < (size - 7); i++) {
		if (matrix[i][6] == last) return false;
		if (last == 1) last = 0;
		else last = 1;
	}



	return true;
}

int decodeRegularByte(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - (offsety+1)][size - offsetx] << 5);
	tempbyte |= (matrix[size - (offsety+1)][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - (offsety+2)][size - offsetx] << 3);
	tempbyte |= (matrix[size - (offsety+2)][size - offsetx - 1] << 2);
	tempbyte |= (matrix[size - (offsety+3)][size - offsetx] << 1);
	tempbyte |= (matrix[size - (offsety+3)][size - offsetx -1]);
	return tempbyte;
}

int decodeRegularByteSkipAligner(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - (offsety + 1)][size - offsetx] << 5);
	tempbyte |= (matrix[size - (offsety + 1)][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - (offsety + 7)][size - offsetx] << 3);
	tempbyte |= (matrix[size - (offsety + 7)][size - offsetx - 1] << 2);
	tempbyte |= (matrix[size - (offsety + 8)][size - offsetx] << 1);
	tempbyte |= (matrix[size - (offsety + 8)][size - offsetx - 1]);
	return tempbyte;
}

int decodeReverseByte(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx - 1] << 2);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx - 1]);
	return tempbyte;
}

int decodeRightToLeft(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 2] << 3);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 3] << 2);
	tempbyte |= (matrix[size - offsety][size - offsetx - 2] << 1);
	tempbyte |= (matrix[size - offsety][size - offsetx - 3]);
	return tempbyte;
}

int decodeLeftToRight(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 2] << 3);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 3] << 2);
	tempbyte |= (matrix[size - offsety][size - offsetx - 2] << 1);
	tempbyte |= (matrix[size - offsety][size - offsetx - 3]);
	return tempbyte;
}

int decodeCornerToLeft(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 6);
	tempbyte |= (matrix[size - offsety][size - offsetx - 2] << 5);
	tempbyte |= (matrix[size - offsety][size - offsetx - 3] << 4);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 2] << 3);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 3] << 2);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx - 2] << 1);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx - 3]);

	return tempbyte;
}

int decodeAroundAlignerLeft(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx] << 6);
	tempbyte |= (matrix[size - offsety - 2][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety - 3][size - offsetx] << 4);
	tempbyte |= (matrix[size - offsety - 4][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety - 5][size - offsetx + 1] << 2);
	tempbyte |= (matrix[size - offsety - 5][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety - 6][size - offsetx + 1]);
	return tempbyte;
}

int decodeTetrisUp(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety - 2][size - offsetx + 1] << 4);
	tempbyte |= (matrix[size - offsety - 2][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety - 3][size - offsetx + 1] << 2);
	tempbyte |= (matrix[size - offsety - 3][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety - 4][size - offsetx + 1]);
	return tempbyte;
}

int decodeModTetrisUp(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety - 2][size - offsetx + 1] << 4);
	tempbyte |= (matrix[size - offsety - 2][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety - 3][size - offsetx + 1] << 2);
	tempbyte |= (matrix[size - offsety - 3][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety - 5][size - offsetx + 1]);
	return tempbyte;
}

int decodeTopRightToLeft(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 2] << 3);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 2);
	tempbyte |= (matrix[size - offsety][size - offsetx - 2] << 1);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 1]);
	return tempbyte;
}

int decodeModTetrisDown(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx + 1] << 4);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx + 1] << 2);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety + 5][size - offsetx + 1]);
	return tempbyte;
}

int decodeTetrisDown(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx + 1] << 4);
	tempbyte |= (matrix[size - offsety + 2][size - offsetx] << 3);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx + 1] << 2);
	tempbyte |= (matrix[size - offsety + 3][size - offsetx] << 1);
	tempbyte |= (matrix[size - offsety + 4][size - offsetx + 1]);
	return tempbyte;
}


int decodeBottomRightToLeft(int ** matrix, int size, int offsetx, int offsety) {
	int tempbyte = 0;
	tempbyte |= (matrix[size - offsety][size - offsetx] << 7);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx + 1] << 6);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx] << 5);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 1] << 4);
	tempbyte |= (matrix[size - offsety + 1][size - offsetx - 2] << 3);
	tempbyte |= (matrix[size - offsety][size - offsetx - 1] << 2);
	tempbyte |= (matrix[size - offsety][size - offsetx - 2] << 1);
	tempbyte |= (matrix[size - offsety - 1][size - offsetx - 1]);
	return tempbyte;
}




string qrDecode(int** matrix, int size, int length) { /* Recebe a matriz de bits, devolve uma string com o conteudo do codigo QR */
	string output = "";

	for (int i = 7; i < 15; i += 4) {
		output += (char)decodeRegularByte(matrix, size, 1, i);
		if (length == output.length()) return output;
	}

	output += (char)decodeRightToLeft(matrix, size, 1, 15);
	if (length == output.length()) return output;

	for (int i = 14; i > 2; i -= 4) {
		output += (char)decodeReverseByte(matrix, size, 3, i);
		if (length == output.length()) return output;
	}

	output += (char)decodeLeftToRight(matrix, size, 3, 2);
	if (length == output.length()) return output;

	output += (char)decodeRegularByteSkipAligner(matrix, size, 5, 3);
	if (length == output.length()) return output;

	output += (char)decodeRegularByte(matrix, size, 5, 12);
	if (length == output.length()) return output;

	output += (char)decodeCornerToLeft(matrix, size, 5, 16);
	if (length == output.length()) return output;

	output += (char)decodeReverseByte(matrix, size, 7, 13);
	if (length == output.length()) return output;

	output += (char)decodeReverseByte(matrix, size, 7, 4);
	if (length == output.length()) return output;
	
	output += (char)decodeRegularByte(matrix, size, 9, 1);
	if (length == output.length()) return output;

	output += (char)decodeAroundAlignerLeft(matrix, size, 10, 5);
	if (length == output.length()) return output;

	output += (char)decodeTetrisUp(matrix, size, 10, 11);
	if (length == output.length()) return output;

	output += (char)decodeModTetrisUp(matrix, size, 10, 15);
	if (length == output.length()) return output;

	output += (char)decodeTetrisUp(matrix, size, 10, 20);
	if (length == output.length()) return output;

	output += (char)decodeTopRightToLeft(matrix, size, 10, 24);
	if (length == output.length()) return output;

	output += (char)decodeModTetrisDown(matrix, size, 12, 23);
	if (length == output.length()) return output;

	for (int i = 18; i > 2; i -= 4) {
		output += (char)decodeTetrisDown(matrix, size, 12, i);
		if (length == output.length()) return output;
	}

	output += (char)decodeBottomRightToLeft(matrix, size, 12, 2);
	if (length == output.length()) return output;

	for (int i = 3; i < 15; i += 4) {
		output += (char)decodeTetrisUp(matrix, size, 14, i);
		if (length == output.length()) return output;
	}

	output += (char)decodeModTetrisUp(matrix, size, 14, 15);
	if (length == output.length()) return output;

	output += (char)decodeTetrisUp(matrix, size, 14, 20);
	if (length == output.length()) return output;
	
	output += (char)decodeTopRightToLeft(matrix, size, 14, 24);
	if (length == output.length()) return output;

	output += (char)decodeModTetrisDown(matrix, size, 16, 23);
	if (length == output.length()) return output;

	output += (char)decodeTetrisDown(matrix, size, 16, 18);
	if (length == output.length()) return output;


	return output;
}

Point2f interseccao(Point2f A1, Point2f A2, Point2f B1, Point2f B2) { /* Calcular ponto de interseccao de duas linhas, dados dois pontos pertencentes a cada linha */

	/*double x = ((A1.x*A2.y - A1.y*A2.x)*(B1.x - B2.x) - (A1.x - A2.x)*(B1.x*B2.y - B1.y*B2.x)) / ((A1.x - A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.x));		Not very clean...
	double y = ((A1.x*A2.y - A1.y*A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.y - B1.y*B2.x)) / ((A1.x - A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.x));
	cout << "X: " << x;
	return Point2f(((A1.x*A2.y - A1.y*A2.x)*(B1.x - B2.x) - (A1.x - A2.x)*(B1.x*B2.y - B1.y*B2.x)) / ((A1.x - A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.x)),
			       ((A1.x*A2.y - A1.y*A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.y - B1.y*B2.x)) / ((A1.x - A2.x)*(B1.y - B2.y) - (A1.y - A2.y)*(B1.x*B2.x))); */ 


	Point2f r(A2 - A1);
	Point2f s(B2 - B1);

	double t = ((B1.x - A1.x) * s.y - (B1.y - A1.y) * s.x) / (r.x * s.y - r.y*s.x);

	return A1 + t*r;
}

int getFarthestIndex(vector<Point> points, Point2f center) {
	int farthestindex = -1, farthestval = -1;
	for (int i = 0; i < points.size(); i++) {
		double actualdist = distancia(center, points[i]);
		if (actualdist > farthestval) {
			farthestval = actualdist;
			farthestindex = i;
		}
	}
	return farthestindex;
}

double declive(Point2f A, Point2f B) {
	double dy = B.y - A.y;
	double dx = B.x - A.x;
	return dy / dx;
}

bool isSquare(vector<Point> poly) {
	double w = distancia(poly[0], poly[1]);
	double h = distancia(poly[1], poly[2]);
	double ratio = w / (1.0 * h);
	if (ratio < 2 && ratio > 0) {
		return true;
	}

	return false;
}

Point2f pontoMedio(Point2f A, Point2f B) {
	return Point2f((A.x + B.x) / 2, (A.y + B.y) / 2);
}

double distanciaPerpendicular(Point2f h1, Point2f h2, Point2f p) {
	double k = declive(h1, h2);
	return (-k * p.x + p.y + ((k * h1.x) - h1.y)) / sqrt((k*k) + 1);
}


void drawAndReturn(Mat img, Mat detection, Mat output, bool init) {
	imshow("Live feed", img);
	if (!init) imshow("Extracted", detection);
	waitKey(1);
}

int main(int argc, char** argv)
{
	VideoCapture camera(0);
	Mat original;
	Mat cinza;
	Mat source;
	Mat detection;
	int align1;
	int align2;
	int align3;
	bool detection_initialized = false;
	int detect_counter = 0;
	string last_decoded = "";
	while (1) {
		align1 = 0;
		align2 = 0;
		align3 = 0;
		camera >> source;
		original = source.clone();
		cinza = original.clone();
		Mat detection(original.size(), CV_8UC3, Scalar(0,0,0));
		Mat output(500,500, CV_8UC3, Scalar(0, 0, 0));
		cvtColor(cinza, cinza, CV_RGB2GRAY);
		Canny(cinza, cinza, 100, 200);
		vector<vector<Point> > contornos;
		vector<Vec4i> hierarchy;
		findContours(cinza, contornos, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		Scalar cor(0, 0, 255);


		vector<Moments> momentos(contornos.size());
		vector<Point2f> centros(contornos.size());
		vector<Point> pontos_poligono, pp1, pp2, pp3, ppr1, ppr2, ppSupEsquerdo, ppInfEsquerdo, ppSupDireito;
		for (int i = 0; i < contornos.size(); i++)
		{
			momentos[i] = moments(contornos[i], false);
		}

		for (int i = 0; i < contornos.size(); i++)
		{
			centros[i] = Point2f(momentos[i].m10 / momentos[i].m00, momentos[i].m01 / momentos[i].m00); // Calcular centros de massa
		}


		for (int i = 0; i < contornos.size(); i++) {

			if (checkDepth(hierarchy, i) == 5) { // Verificar o número de filhos do contorno
				approxPolyDP(contornos[i], pontos_poligono, 0.1*arcLength(contornos[i], true), true); // Aproximar um contorno a um polígono, com imprecisão máxima de 10% do perímetro
				if (pontos_poligono.size() != 4) continue;  // Ignorar poligonos com mais que 4 vértices
				if (!isSquare(pontos_poligono)) continue; // Verificar se o poligono se aproxima a um quadrado
				if (!align1) {
					align1 = i;
					pp1 = pontos_poligono;
				}
				else if (!align2) {
					align2 = i;
					pp2 = pontos_poligono;
				}
				else if (!align3) {
					align3 = i;
					pp3 = pontos_poligono;
				}
				if (align3) break;
			}
		}

		if (!align3) {
			drawAndReturn(original, detection, output, detection_initialized);
			detection_initialized = true;
			continue;
		}

		/* Descobrir entre que pontos de alinhamento está a hipotenusa, o restante ponto será o canto superior */

		double dist12, dist23, dist31;
		dist12 = distancia(centros[align1], centros[align2]);
		dist23 = distancia(centros[align2], centros[align3]);
		dist31 = distancia(centros[align3], centros[align1]);

		double maxDist = max({ dist12, dist23, dist31 });

		int cantoSupEsquerdo = -1;
		int restante1 = -1, restante2 = -1;
		if (maxDist == dist12) {
			cantoSupEsquerdo = align3; 
			ppSupEsquerdo = pp3;
			restante1 = align1;
			ppr1 = pp1;
			restante2 = align2;
			ppr2 = pp2;
		}
		else if (maxDist == dist23) {
			cantoSupEsquerdo = align1;
			ppSupEsquerdo = pp1;
			restante1 = align2;
			ppr1 = pp2;
			restante2 = align3;
			ppr2 = pp3;
		}
		else if (maxDist == dist31) {
			cantoSupEsquerdo = align2;
			ppSupEsquerdo = pp2;
			restante1 = align1;
			ppr1 = pp1;
			restante2 = align3;
			ppr2 = pp3;
		}


		int cantoInfEsq = -1, cantoSupDireito = -1;

		double dP = distanciaPerpendicular(centros[restante1], centros[restante2], centros[cantoSupEsquerdo]);
		double k = declive(centros[restante1], centros[restante2]);

		if (dP < 0 && k < 0) { cantoInfEsq = restante1; cantoSupDireito = restante2; ppInfEsquerdo = ppr1; ppSupDireito = ppr2; }
		else if (dP < 0 && k > 0) { cantoInfEsq = restante2; cantoSupDireito = restante1; ppInfEsquerdo = ppr2; ppSupDireito = ppr1; }
		else if (dP > 0 && k < 0) { cantoInfEsq = restante2; cantoSupDireito = restante1; ppInfEsquerdo = ppr2; ppSupDireito = ppr1; }
		else if (dP > 0 && k > 0) { cantoInfEsq = restante1; cantoSupDireito = restante2; ppInfEsquerdo = ppr1; ppSupDireito = ppr2; }

		Point2f centro = pontoMedio(centros[restante1], centros[restante2]);
		
		int farthestSupEsquerdo = getFarthestIndex(ppSupEsquerdo, centro);
		int farthestInfEsquerdo = getFarthestIndex(ppInfEsquerdo, centro);
		int farthestSupDireito = getFarthestIndex(ppSupDireito, centro);
		int secondPointInfEsquerdo = farthestInfEsquerdo + 1;
		if (secondPointInfEsquerdo == 4) secondPointInfEsquerdo = 0;
		int secondPointSupDireito = farthestSupDireito - 1;
		if (secondPointSupDireito == -1) secondPointSupDireito = 3;


		Point2f oposto = interseccao(ppInfEsquerdo[farthestInfEsquerdo], ppInfEsquerdo[secondPointInfEsquerdo], ppSupDireito[farthestSupDireito], ppSupDireito[secondPointSupDireito]);


		circle(detection, centro, 2, Scalar(255, 255, 255), 2);
		arrowedLine(detection, ppInfEsquerdo[farthestInfEsquerdo], oposto,  Scalar(0, 255, 0), 1);
		arrowedLine(detection, ppSupDireito[farthestSupDireito], oposto, Scalar(0, 255, 0), 1);
		arrowedLine(original, ppInfEsquerdo[farthestInfEsquerdo], oposto, Scalar(0, 255, 0), 1);
		arrowedLine(original, ppSupDireito[farthestSupDireito], oposto, Scalar(0, 255, 0), 1);
		line(detection, ppInfEsquerdo[farthestInfEsquerdo], ppSupEsquerdo[farthestSupEsquerdo],  Scalar(0, 255, 0), 1);
		line(detection, ppSupDireito[farthestSupDireito], ppSupEsquerdo[farthestSupEsquerdo], Scalar(0, 255, 0), 1);


		drawContours(original, contornos, hierarchy[cantoSupEsquerdo][2], Scalar(0,255,0), 2);
		drawContours(detection, contornos, hierarchy[cantoSupEsquerdo][2], Scalar(0, 255, 0), 2);

		drawContours(original, contornos, hierarchy[cantoInfEsq][2], Scalar(0, 0, 255), 2);
		drawContours(detection, contornos, hierarchy[cantoInfEsq][2], Scalar(0, 0, 255), 2);

		drawContours(original, contornos, hierarchy[cantoSupDireito][2], Scalar(0, 255, 255), 2);
		drawContours(detection, contornos, hierarchy[cantoSupDireito][2], Scalar(0, 255, 255), 2);

		Point2f source_coords[] = {ppSupEsquerdo[farthestSupEsquerdo], ppSupDireito[farthestSupDireito], oposto, ppInfEsquerdo[farthestInfEsquerdo]};
		Point2f destination_coords[] = {Point2f(0.0,0.0), Point2f(500, 0), Point2f(500, 500), Point2f(0, 500)};

		warpPerspective(source, output, getPerspectiveTransform(source_coords, destination_coords), Size(500, 500));
		cvtColor(output, output, CV_RGB2GRAY);
		Mat thres128 = output.clone();
		threshold(output, thres128, 128, 255, THRESH_BINARY);
		int** matrix; 
		//= getDataMatrix(thres128, QRSIZE);

		

		bool alignment_ok = false;

		Mat ideal = output.clone();
		for (int i = 64; i <= 192; i += 16) {
			threshold(output, ideal, i, 255, THRESH_BINARY);
			matrix = getDataMatrix(ideal, QRSIZE);
			if (checkAlignmentAndTiming(matrix, QRSIZE)) {
				alignment_ok = true;
				break;
			}
			for (int i = 0; i < 25; i++) delete[] matrix[i];
			delete[] matrix;

		}

		bool success = false;
		if (alignment_ok) {
			unmask(matrix, QRSIZE);
			int encoding = getEncoding(matrix, QRSIZE);
			if (encoding == 4) {
				int length = getLength(matrix, QRSIZE, encoding);
				if (length <= 32) {
					string decoded = qrDecode(matrix, QRSIZE, length);
					if (decoded == last_decoded) {
						if (detect_counter == 0) {
							printf("\aRead (%2d) bytes - [%s]\n", length, decoded.c_str());
							detect_counter++;
							success = true;
							if (decoded.find("http://") == 0 || decoded.find("https://") == 0) {
								string command = "start chrome " + decoded;
								system(command.c_str());
							}
						}
					}
					else {
						last_decoded = decoded;
						detect_counter = 0;
					}
				}
			}
		}
	//		for (int i = 0; i < 25; i++) delete[] matrix[i];
	//	delete[] matrix;

		cvtColor(ideal, ideal, CV_GRAY2RGB);
		for (int i = 0; i < QRSIZE; i++) line(ideal, Point(0, (ideal.rows / QRSIZE) * (i + 1)), Point(ideal.cols - 1, (ideal.rows / QRSIZE) * (i + 1)), Scalar(0, 0, 255));
		for (int i = 0; i < QRSIZE; i++) line(ideal, Point((ideal.cols / QRSIZE) * (i + 1), 0), Point((ideal.cols / QRSIZE) * (i + 1), ideal.rows - 1), Scalar(0, 0, 255));
		copyMakeBorder(thres128, thres128, 20, 20, 20, 20, BORDER_CONSTANT, Scalar(255, 255, 255));
		copyMakeBorder(ideal, ideal, 20, 20, 20, 20, BORDER_CONSTANT, Scalar(255, 255, 255));
		

		

		imshow("Live feed", original);
		imshow("Extracted", detection);
		imshow("Output128", thres128);
		if (success) imshow("Found it", ideal);
		detection_initialized = true;
		waitKey(1);
	}
	
	return 0;
}


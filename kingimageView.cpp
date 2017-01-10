#include "stdafx.h"
#include "kingimage.h"
#include <sstream>
#include <vector>
#include <fstream>
#include <iomanip> 
#include <cmath>
#include <algorithm>
#include "kingimageDoc.h"
#include "kingimageView.h"

using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

vector <int> labels;
vector <int> valuesN1;
vector <vector <MotionPattern>> frame1;
vector <vector <MotionPattern>> frame2;

IMPLEMENT_DYNCREATE(CKingimageView, CScrollView)

BEGIN_MESSAGE_MAP(CKingimageView, CScrollView)
	//{{AFX_MSG_MAP(CKingimageView)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_PROCESS, OnProcess)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_PROCESS_PATTERN, CKingimageView::ManuelGrouping)
	ON_COMMAND(ID_PROCESS_K, CKingimageView::KMeans)
	ON_COMMAND(ID_PROCESS_N1, CKingimageView::NN1)
	ON_COMMAND(ID_PROCESS_N2, CKingimageView::NN2)
	ON_COMMAND(ID_PROCESS_N3, CKingimageView::NN3)
	ON_COMMAND(ID_PROCESS_N4, CKingimageView::NN4)
	ON_COMMAND(ID_PROCESS_ERRORRATE, CKingimageView::ErrorRate)
	ON_COMMAND(ID_PROCESS_2, CKingimageView::ManuelGrouping2)
	ON_COMMAND(ID_PROCESS_F1, CKingimageView::RetriveF1)
	ON_COMMAND(ID_PROCESS_F2, CKingimageView::RetriveF2)
	ON_COMMAND(ID_PROCESS_MOTIONDETECTION, CKingimageView::MotionDetection)
	ON_COMMAND(ID_PROCESS_EXTRACTDIFFERENCE, CKingimageView::Difference)
END_MESSAGE_MAP()

void CKingimageView::Difference()
{
	int diff;

	for (int i = 1; i < frame1.size() - 1; i++)
	{
		for (int j = 1; j < frame1.at(0).size() - 1; j++)
		{
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					diff = abs(frame1.at(i).at(j).vector.at(x).at(y) - frame2.at(i).at(j).vector.at(x).at(y));

					if (diff > 20)
					{
						frame1.at(i).at(j).vector.at(x).at(y) = 0;
						frame1.at(i).at(j).vector.at(x).at(y) = 0;
					}
				}
			}
		}
	}

	ReDraw();
}

void CKingimageView::MotionDetection()
{
	int diff;

	for (int i = 1; i < frame1.size() - 1; i++)
	{
		for (int j = 1; j < frame1.at(0).size() - 1; j++)
		{
			diff = abs(frame1.at(i).at(j).avg - frame2.at(i).at(j).avg);

			if (diff > 5)
			{
				for (int x = 0; x < 8; x++)
				{
					frame1.at(i).at(j).vector.at(x).at(3) = 255;
					frame1.at(i).at(j).vector.at(x).at(4) = 255;
				}
			}
		}
	}

	ReDraw();
}

void CKingimageView::ReDraw()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int rowCounter = 0, columnCounter = 0, startingPoint;
	
	for (int i = 0; i < iHeight; i = i + 8)
	{
		rowCounter = 0;
		for (int j = 0; j < iWidth; j = j + 8)
		{
			for (int n = 0; n < 8; n++)
			{
				for (int m = 0; m < 8; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);

					pImg[startingPoint] = frame1.at(columnCounter).at(rowCounter).vector.at(n).at(m);
					pImg[startingPoint + 1] = frame1.at(columnCounter).at(rowCounter).vector.at(n).at(m);
					pImg[startingPoint + 2] = frame1.at(columnCounter).at(rowCounter).vector.at(n).at(m);
				}
			}

			rowCounter++;
		}
		columnCounter++;
	}

	OnDraw(GetDC());
}

void CKingimageView::RetriveF1()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int R, G, B, intensity, startingPoint;
	double avg;

	for (int i = 0; i < iHeight; i++)
	{
		for (int j = 0; j < iWidth; j++)
		{
			R = pImg[i * iWidth * 3 + j * 3];
			G = pImg[i * iWidth * 3 + j * 3 + 1];
			B = pImg[i * iWidth * 3 + j * 3 + 2];

			intensity = (R + G + B) / 3;

			pImg[i * iWidth * 3 + j * 3] = intensity;
			pImg[i * iWidth * 3 + j * 3 + 1] = intensity;
			pImg[i * iWidth * 3 + j * 3 + 2] = intensity;
		}
	}

	for (int i = 0; i < iHeight; i = i + 8)
	{
		vector <MotionPattern> row;

		for (int j = 0; j < iWidth; j = j + 8)
		{
			MotionPattern mp;
			avg = 0;

			for (int n = 0; n < 8; n++)
			{
				for (int m = 0; m < 8; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					mp.vector.at(n).at(m) = pImg[startingPoint];
					avg = avg + pImg[startingPoint];
				}
			}
			mp.avg = avg / 64.0;

			row.push_back(mp);
		}
		frame1.push_back(row);
	}

	OnDraw(GetDC());
}

void CKingimageView::RetriveF2()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int R, G, B, intensity, startingPoint;
	double avg;

	for (int i = 0; i < iHeight; i++)
	{
		for (int j = 0; j < iWidth; j++)
		{
			R = pImg[i * iWidth * 3 + j * 3];
			G = pImg[i * iWidth * 3 + j * 3 + 1];
			B = pImg[i * iWidth * 3 + j * 3 + 2];

			intensity = (R + G + B) / 3;

			pImg[i * iWidth * 3 + j * 3] = intensity;
			pImg[i * iWidth * 3 + j * 3 + 1] = intensity;
			pImg[i * iWidth * 3 + j * 3 + 2] = intensity;
		}
	}

	for (int i = 0; i < iHeight; i = i + 8)
	{
		vector <MotionPattern> row;

		for (int j = 0; j < iWidth; j = j + 8)
		{
			MotionPattern mp;
			avg = 0;

			for (int n = 0; n < 8; n++)
			{
				for (int m = 0; m < 8; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					mp.vector.at(n).at(m) = pImg[startingPoint];
					avg = avg + pImg[startingPoint];
				}
			}
			mp.avg = avg / 64.0;

			row.push_back(mp);
		}

		frame2.push_back(row);
	}

	OnDraw(GetDC());
}

void CKingimageView::ManuelGrouping2()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int avg, startingPoint;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			avg = 0;

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					avg = avg + pImg[startingPoint];
				}
			}

			avg = avg / 16;

			if (avg >= 0 && avg < 125)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 0;
						pImg[startingPoint + 1] = 0;
						pImg[startingPoint + 2] = 0;
					}
				}
			}
			else if (avg >= 125 && avg < 175)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 128;
						pImg[startingPoint + 1] = 128;
						pImg[startingPoint + 2] = 128;
					}
				}
			}
			else if (avg >= 175 && avg <= 255)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 255;
						pImg[startingPoint + 1] = 255;
						pImg[startingPoint + 2] = 255;
					}
				}
			}
			else printMessage(-999);
		}
	}

	OnDraw(GetDC());
}

void CKingimageView::ErrorRate()
{
	CKingimageDoc* pDoc = GetDocument();
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int valueN1, valueT1, startingPoint;
	unsigned int counter = 0;
	double avg = 0.0;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					valueT1 = pImg[startingPoint];
					valueN1 = valuesN1.at(counter);
					counter++;
					avg = avg + pow(valueT1 - valueN1, 2);
				}
			}
		}
	}

	printMessage(sqrt(avg) / counter);
}

void CKingimageView::NN1()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int testValue, trainValue, minDistanceLoc, label;
	double distance;
	vector <double> distanceVector;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			distanceVector.clear();

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					distance = 0;

					for (int n = 0; n < 4; n++)
					{
						for (int m = 0; m < 4; m++)
						{
							testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
							trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);

							distance = distance + pow(pImg[testValue] - pImg[trainValue], 2);
						}
					}

					distance = sqrt(distance);
					distanceVector.push_back(distance);
				}
			}

			minDistanceLoc = findMin(distanceVector);
			label = labels.at(minDistanceLoc);

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					valuesN1.push_back(pImg[testValue]);

					pImg[testValue] = label;
					pImg[testValue + 1] = label;
					pImg[testValue + 2] = label;
				}
			}
		}
	}

	OnDraw(GetDC());
}

void CKingimageView::NN2()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int testValue, trainValue, minDistanceLoc, counter;
	double distance;
	vector <double> distanceVector;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			distanceVector.clear();

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					distance = 0;

					for (int n = 0; n < 4; n++)
					{
						for (int m = 0; m < 4; m++)
						{
							testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
							trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);

							distance = distance + pow(pImg[testValue] - pImg[trainValue], 2);
						}
					}

					distance = sqrt(distance);
					distanceVector.push_back(distance);
				}
			}

			minDistanceLoc = findMin(distanceVector);
			counter = 0;
			
			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					if (counter == minDistanceLoc)
					{
						for (int n = 0; n < 4; n++)
						{
							for (int m = 0; m < 4; m++)
							{
								testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
								trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);

								pImg[testValue] = pImg[trainValue];
								pImg[testValue + 1] = pImg[trainValue + 1];
								pImg[testValue + 2] = pImg[trainValue + 2];
							}
						}
					}

					counter++;
				}
			}
		}
	}

	OnDraw(GetDC());
}

void CKingimageView::NN3()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int testValue, trainValue, minDistanceLoc, counter, avg;
	double distance;
	vector <double> distanceVector;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			distanceVector.clear();

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					distance = 0;

					for (int n = 0; n < 4; n++)
					{
						for (int m = 0; m < 4; m++)
						{
							testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
							trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);

							distance = distance + pow(pImg[testValue] - pImg[trainValue], 2);
						}
					}

					distance = sqrt(distance);
					distanceVector.push_back(distance);
				}
			}

			minDistanceLoc = findMin(distanceVector);
			counter = 0;

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					if (counter == minDistanceLoc)
					{
						avg = 0;

						for (int n = 0; n < 4; n++)
						{
							for (int m = 0; m < 4; m++)
							{
								trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);
								avg = avg + pImg[trainValue];
							}
						}

						avg = avg / 16;
					}

					counter++;
				}
			}

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);

					pImg[testValue] = avg;
					pImg[testValue + 1] = avg;
					pImg[testValue + 2] = avg;
				}
			}
		}
	}

	OnDraw(GetDC());
}

void CKingimageView::NN4()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int testValue, trainValue, minDistanceLoc, label, classAvg, counter, pixelCounter;
	double distance;
	vector <double> distanceVector;

	for (int i = 0; i < iHeight / 2; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			distanceVector.clear();

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					distance = 0;

					for (int n = 0; n < 4; n++)
					{
						for (int m = 0; m < 4; m++)
						{
							testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
							trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);

							distance = distance + pow(pImg[testValue] - pImg[trainValue], 2);
						}
					}

					distance = sqrt(distance);
					distanceVector.push_back(distance);
				}
			}

			minDistanceLoc = findMin(distanceVector);
			label = labels.at(minDistanceLoc);
			classAvg = 0, counter = 0, pixelCounter = 0;

			for (int x = iHeight / 2; x < iHeight; x = x + 4)
			{
				for (int y = 0; y < iWidth; y = y + 4)
				{
					if (label == labels.at(counter))
					{
						for (int n = 0; n < 4; n++)
						{
							for (int m = 0; m < 4; m++)
							{
								trainValue = (x * iWidth * 3) + (n * iWidth * 3) + (y * 3) + (m * 3);
								classAvg = classAvg + pImg[trainValue];
								pixelCounter++;
							}
						}
					}

					counter++;
				}
			}

			classAvg = classAvg / pixelCounter;

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					testValue = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);

					pImg[testValue] = classAvg;
					pImg[testValue + 1] = classAvg;
					pImg[testValue + 2] = classAvg;
				}
			}
		}
	}

	OnDraw(GetDC());
}

void CKingimageView::KMeans()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int startingPoint;
	vector <pattern> trainingVectors;
	double avg;

	for (int i = iHeight / 2; i < iHeight; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			pattern v;
			avg = 0;

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					v.vector.at(n).at(m) = pImg[startingPoint];
					avg = avg + pImg[startingPoint];
				}
			}
			v.avg = avg / 16.0;

			trainingVectors.push_back(v);
		}
	}

	vector <pattern> cluster1, cluster2, cluster3;
	pattern clusterCenter1, clusterCenter2, clusterCenter3;
	double distance1 = 0, distance2 = 0, distance3 = 0, smallestD;
	double distances[3] = { distance1 , distance2 , distance3 };
	int smallestIndex;

	int cc1 = rand() % trainingVectors.size();
	clusterCenter1 = trainingVectors.at(cc1);
	clusterCenter1.center = true;
	cluster1.push_back(clusterCenter1);

	int cc2 = rand() % trainingVectors.size();
	clusterCenter2 = trainingVectors.at(cc2);
	clusterCenter2.center = true;
	cluster2.push_back(clusterCenter2);

	int cc3 = rand() % trainingVectors.size();
	clusterCenter3 = trainingVectors.at(cc3);
	clusterCenter3.center = true;
	cluster3.push_back(clusterCenter3);

	for (int x = 0; x < 5; x++)
	{
		for (unsigned int i = 0; i < trainingVectors.size(); i++)
		{
			if (trainingVectors.at(i).center == false)
			{
				distances[0] = findDistance(trainingVectors.at(i), clusterCenter1);
				distances[1] = findDistance(trainingVectors.at(i), clusterCenter2);
				distances[2] = findDistance(trainingVectors.at(i), clusterCenter3);

				smallestD = *min_element(distances, distances + 3);

				if (smallestD == distances[0]) smallestIndex = 0;
				else if (smallestD == distances[1]) smallestIndex = 1;
				else if (smallestD == distances[2]) smallestIndex = 2;
				else printMessage(-9999);

				if (smallestIndex == 0)
				{
					trainingVectors.at(i).cluster = 1;
					cluster1.push_back(trainingVectors.at(i));
				}
				else if (smallestIndex == 1) 
				{
					trainingVectors.at(i).cluster = 2;
					cluster2.push_back(trainingVectors.at(i));
				}
				else if (smallestIndex == 2)
				{
					trainingVectors.at(i).cluster = 3;
					cluster3.push_back(trainingVectors.at(i));
				}
				else printMessage(-9999);
			}
		}

		clusterCenter1 = updateCenter(cluster1);
		clusterCenter2 = updateCenter(cluster2);
		clusterCenter3 = updateCenter(cluster3);

		cluster1.clear();
		cluster2.clear();
		cluster3.clear();

		cluster1.push_back(clusterCenter1);
		cluster2.push_back(clusterCenter2);
		cluster3.push_back(clusterCenter3);
	}

	int counter = 0, label;

	for (int i = iHeight / 2; i < iHeight; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			label = trainingVectors.at(counter).cluster;

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);

					if (label == 1)
					{
						pImg[startingPoint] = 255;
						pImg[startingPoint + 1] = 255;
						pImg[startingPoint + 2] = 255;
					}
					else if (label == 2) 
					{
						pImg[startingPoint] = 128;
						pImg[startingPoint + 1] = 128;
						pImg[startingPoint + 2] = 128;
					}
					else if (label == 3)
					{
						pImg[startingPoint] = 0;
						pImg[startingPoint + 1] = 0;
						pImg[startingPoint + 2] = 0;
					}
					else printMessage(-9999);
				}
			}

			counter++;
		}
	}

	OnDraw(GetDC());
}

pattern CKingimageView::updateCenter(vector <pattern> cluster)
{
	pattern newCenter;
	double classAvg = 0;

	for (unsigned int i = 0; i < cluster.size(); i++)
	{
		if (cluster.at(i).center == true) cluster.at(i).center = false;
		classAvg = classAvg + cluster.at(i).avg;
	}

	classAvg = classAvg / (cluster.size() * 1.0);
	double min = 999;
	int newCenterIndex;

	for (unsigned int i = 0; i < cluster.size(); i++)
	{
		if (abs(classAvg - cluster.at(i).avg) < min)
		{
			min = abs(classAvg - cluster.at(i).avg);
			newCenterIndex = i;
		}
	}

	for (unsigned int i = 0; i < cluster.size(); i++)
	{
		if (i == newCenterIndex)
		{
			cluster.at(i).center = true;
			newCenter = cluster.at(i);
		}
	}

	return newCenter;
}

double CKingimageView::findDistance(pattern p1, pattern p2)
{
	double distance = 0.0;

	for (int n = 0; n < 4; n++)
	{
		for (int m = 0; m < 4; m++)
		{
			distance = distance + pow(p1.vector.at(n).at(m) - p2.vector.at(n).at(m), 2);
		}
	}

	return sqrt(distance);
}

void CKingimageView::ManuelGrouping()
{
	CKingimageDoc* pDoc = GetDocument();
	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int avg, startingPoint;

	for (int i = iHeight / 2; i < iHeight; i = i + 4)
	{
		for (int j = 0; j < iWidth; j = j + 4)
		{
			avg = 0;

			for (int n = 0; n < 4; n++)
			{
				for (int m = 0; m < 4; m++)
				{
					startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
					avg = avg + pImg[startingPoint];
				}
			}

			avg = avg / 16;

			if (avg >= 0 && avg < 125)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 0;
						pImg[startingPoint + 1] = 0;
						pImg[startingPoint + 2] = 0;
					}
				}

				labels.push_back(0);
			}
			else if (avg >= 125 && avg < 175)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 128;
						pImg[startingPoint + 1] = 128;
						pImg[startingPoint + 2] = 128;
					}
				}

				labels.push_back(128);
			}
			else if (avg >= 175 && avg <= 255)
			{
				for (int n = 0; n < 4; n++)
				{
					for (int m = 0; m < 4; m++)
					{
						startingPoint = (i * iWidth * 3) + (n * iWidth * 3) + (j * 3) + (m * 3);
						pImg[startingPoint] = 255;
						pImg[startingPoint + 1] = 255;
						pImg[startingPoint + 2] = 255;
					}
				}

				labels.push_back(255);
			}
			else printMessage(-999);
		}
	}

	OnDraw(GetDC());
}

CKingimageView::CKingimageView()
{
	// TODO: add construction code here

}

CKingimageView::~CKingimageView()
{
}

BOOL CKingimageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

void CKingimageView::OnDraw(CDC* pDC)
{
	CKingimageDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
//	pDC->SetStretchBltMode(COLORONCOLOR);
//	int x,y,bytes;
	if (pDoc->imagetype==PCX)
		(pDoc->_pcx)->draw(pDC);
	if (pDoc->imagetype==BMP)
		(pDoc->_bmp)->draw(pDC);
	if (pDoc->imagetype==GIF)
		(pDoc->_gif)->draw(pDC);
	if (pDoc->imagetype==JPG)
		(pDoc->_jpg)->draw(pDC);
/*	if (pDoc->imagetype==BMP)
	{
	
	x=((KINGBMP*)(pDoc->_ppicture))->bmpinfo.bmiHeader.biWidth;
	y=((KINGBMP*)(pDoc->_ppicture))->bmpinfo.bmiHeader.biHeight;
	bytes=(pDoc->_ppicture)->needbyte;
	int place=0;
	for (int j=0;j<y;j++)
		for (int i=0;i<x;i++){
//			int color= (pDoc->_bmp)->point[place++] ;
			pDC->SetPixel(i,j,RGB( (pDoc->_picture)->pixel[place+2], (pDoc->_picture)->pixel[place+1] ,(pDoc->_picture)->pixel[place]));
			place+=3;
		}

  /*	if ( (pDoc->_bmp)->bitsperpixel!=24 ){
		CPalette *palette=new CPalette();
		LOGPALETTE palet;
		palet.palVersion=0x300;
		palet.palNumEntries=(pDoc->_bmp)->usedcolor;
		for (int i=0;i<palet.palNumEntries;i++){
			palet.palPalEntry[i].peRed=( (pDoc->_bmp) -> bmpinfo) .bmiColors[i].rgbRed;
			palet.palPalEntry[i].peBlue=( (pDoc->_bmp) -> bmpinfo) .bmiColors[i].rgbBlue;
			palet.palPalEntry[i].peGreen=( (pDoc->_bmp) -> bmpinfo) .bmiColors[i].rgbGreen;
			palet.palPalEntry[i].peFlags=0;//( (pDoc->_bmp) -> bmpinfo) .bmiColors[0].rgbRed;
		}
		BOOL re=palette->CreatePalette(&palet);
		CPalette *pp;
		pp=pDC->SelectPalette(palette,TRUE);
	}
*/
//	::SetDIBitsToDevice(pDC->m_hDC,0,0,x,y,0,0,0,y,(pDoc->_ppicture)->point,&(((KINGBMP*)(pDoc->_ppicture))->bmpinfo),DIB_RGB_COLORS);
	
//	}
	// TODO: add draw code for native data here
}

BOOL CKingimageView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CKingimageView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CKingimageView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

#ifdef _DEBUG
void CKingimageView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CKingimageView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CKingimageDoc* CKingimageView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKingimageDoc)));
	return (CKingimageDoc*)m_pDocument;
}
#endif //_DEBUG

void CKingimageView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	CSize totalSize=CSize(::GetSystemMetrics(SM_CXSCREEN),
		                  ::GetSystemMetrics(SM_CYSCREEN));
	CSize pageSize=CSize(totalSize.cx/2,totalSize.cy/2);
	CSize lineSize=CSize(totalSize.cx/10,totalSize.cy/10);

	SetScrollSizes(MM_TEXT,totalSize,pageSize,lineSize);
}

void CKingimageView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CScrollView::OnMouseMove(nFlags, point);
}

void CKingimageView::OnProcess()
{
	// TODO: Add your command handler code here
	CKingimageDoc* pDoc = GetDocument();


	int iBitPerPixel = pDoc->_bmp->bitsperpixel;
	int iWidth = pDoc->_bmp->width;
	int iHeight = pDoc->_bmp->height;
	BYTE *pImg = pDoc->_bmp->point;

	int Wp = iWidth;
	if (iBitPerPixel == 8)  ////Grey scale 8 bits image
	{
		int r = iWidth % 4;
		int p = (4-r)%4;
		Wp = iWidth + p;
	}
	else if (iBitPerPixel == 24)	// RGB image
	{
		int r = (3*iWidth) % 4;
		int p = (4-r)%4;
		Wp = 3*iWidth + p;
	}

	if (iBitPerPixel == 8)  ////Grey scale 8 bits image
	{
		for(int i=0; i<iHeight; i++)
		for(int j=0; j<iWidth; j++)
		{
			pImg[i*Wp+j] = 255;
		}
	}
	else if(iBitPerPixel == 24)  ////True color 24bits image
	{
		for(int i=0; i<iHeight; i++)
		for(int j=0; j<iWidth; j++)
		{
			pImg[i*Wp+j*3]   = 0;      //B
			// pImg[i*Wp+j*3+1] = 0;      //G
			pImg[i*Wp+j*3+2] = 0;      //R 
		}
	}

	////redraw the screen
	OnDraw(GetDC());
}

int CKingimageView::findMin(vector <double> d)
{
	int counter = 0;
	double min = 9999.0;

	for (unsigned int i = 0; i < d.size(); i++)
	{
		if (d.at(i) < min) min = d.at(i);
	}

	for (unsigned int i = 0; i < d.size(); i++)
	{
		if (min == d.at(i)) return counter;
		counter++;
	} 

	return -1;
}

void CKingimageView::printMessage(double m)
{
	stringstream ss;
	string str;
	char* cchar;
	ss << m;
	str = ss.str();
	cchar = (char*)str.c_str();
	AfxMessageBox(cchar);
}

void CKingimageView::printFile(vector <vector <int>> vector2D)
{
	int height = vector2D.size();
	int width = vector2D.at(0).size();
	ofstream myfile;
	myfile.open("test.txt");
	int i, j;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			myfile << setw(30) << vector2D.at(i).at(j);
		}
		myfile << "\n";
	}

	myfile.close();
}
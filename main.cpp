#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class ART1
{
	using vectorArray = std::vector<std::vector<int> >;
	vectorArray prototypeVectors;

	int vectorMagnitude(const std::vector<int>& vec)
	{
		int count = 0;
		for (int i : vec)
		{
			count += i;
		}
		return count;
	}

	void vectorBitAnd(std::vector<int>& dist, const std::vector<int>& from)
	{
		for (int i = 0; i < dist.size(); i++)
		{
			dist[i] = dist[i] & from[i];
		}
	}

	int createPrototypeVector(const std::vector<int>& vec)
	{
		prototypeVectors.push_back(vec);
		clusterMemberCount.push_back(1);
		sumVectors.push_back(std::vector<int>(vec.size()));
		return prototypeVectors.size() - 1;
	}

	void updatePrototypeVectors(int cluster)
	{
		for (int i = 0; i < prototypeVectors[cluster].size(); i++)
		{
			prototypeVectors[cluster][i] = 1;
			sumVectors[cluster][i] = 0;
		}
		for (int i = 0; i < vectors.size(); i++)
		{
			if (elementsCluster[i] == cluster)
			{
				vectorBitAnd(prototypeVectors[cluster], vectors[i]);
				for (int item = 0; item < vectors[i].size(); item++)
				{
					sumVectors[cluster][item] += vectors[i][item];
				}
			}
		}
	}

protected:
	vectorArray vectors;
	double vigilance;
	double Beta;
	int operationLimit;

public:
	vectorArray sumVectors;
	std::vector<int> elementsCluster;
	std::vector<int> clusterMemberCount;

	virtual void initialize(const vectorArray& vectors, double Beta, double vigilance, int operationLimit)
	{
		this->Beta = Beta;
		this->vigilance = vigilance;
		this->operationLimit = operationLimit;
		this->vectors = vectors;
		elementsCluster.resize(vectors.size(), -1);
	}

	virtual void performART1()
	{
		bool completed = false;
		double magPE, magP, magE, test, result;

		while (!completed)
		{
			completed = true;
			operationLimit--;

			for (int i = 0; i < vectors.size(); i++)
			{
				for (int prototype = 0; prototype < prototypeVectors.size(); prototype++)
				{
					if (clusterMemberCount[prototype] > 0)
					{
						std::vector<int> resultVector(vectors[0].size(), 1);

						vectorBitAnd(resultVector, prototypeVectors[prototype]);
						vectorBitAnd(resultVector, vectors[i]);

						magPE = vectorMagnitude(resultVector);
						magP = vectorMagnitude(prototypeVectors[prototype]);
						magE = vectorMagnitude(vectors[i]);

						result = magPE / (Beta + magP);
						test = magE / (Beta + resultVector.size());
						
						if ((result > test) && (magPE / magE < vigilance))
						{
							int oldPrototype = elementsCluster[i];

							if (oldPrototype != prototype)
							{
								if (oldPrototype > 0)
								{
									clusterMemberCount[oldPrototype]--;
									if (clusterMemberCount[oldPrototype] > 0)
									{
										updatePrototypeVectors(oldPrototype);
									}
								}
								clusterMemberCount[prototype]++;
								elementsCluster[i] = prototype;
								updatePrototypeVectors(prototype);
								completed = false;
							}
						}
					}
				}
			
				if (elementsCluster[i] < 0)
				{
					elementsCluster[i] = createPrototypeVector(vectors[i]);
					completed = false;
				}
			}

			if (operationLimit == 0)
			{
				break;
			}
		}
	}

	virtual ~ART1() = 0;
};

ART1::~ART1() { }

class DataAnalyzer : public ART1
{
	using StringContainer = std::vector<std::string>;
	StringContainer elementNames;

public:

	DataAnalyzer(const StringContainer& elementNames) : elementNames(elementNames) { }

	StringContainer& getElementNames()
	{
		return elementNames;
	}

	std::pair<int, int> makeRecommendation(int elementIndex)
	{
		int bestItem = -1;
		int bestValue = 0;
		for (int i = 0; i < elementNames.size(); i++)
		{
			int currentValue = sumVectors[elementsCluster[elementIndex]][i];
			if ((vectors[elementIndex][i] == 0) && (currentValue > bestValue))
			{
				bestItem = i;
				bestValue = currentValue;
			}
		}
		return { bestItem, bestValue };
	}
};

int main() {
	std::ifstream file("data.txt");
	/*	line 1: elements in a column, elements in a row
		line 2: element's name
		line 3-N: elements {0; 1}
	*/
	int column, row;
	file >> column >> row;
	std::vector<std::string> elements(column);
	std::vector<std::vector<int> > data(row, std::vector<int>(column));
	for (int i = 0; i < column; i++)
	{
		file >> elements[i];
	}
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			file >> data[i][j];
		}
	}
	file.close();

	DataAnalyzer dataAnalyser(elements);
	dataAnalyser.initialize(data, 1.0, 0.9, 50);
	dataAnalyser.performART1();

	for (int i = 0; i < row; i++)
	{
		std::pair<int, int> customer = dataAnalyser.makeRecommendation(i);
		printf("for customer #%d ", i);
		if (customer.first < 0)
		{
			printf("no recommendation could be made\n");
		}
		else
		{
			printf("best item is [%s]\n", elements[customer.first].c_str());
			printf("> bought buy %d of %d customers in the group\n", customer.second,
				dataAnalyser.clusterMemberCount[dataAnalyser.elementsCluster[i]]);
		}
		printf("> customer already owns these items:\n");
		for (int item = 0; item < column; item++)
		{
			if (data[i][item] > 0)
			{
				printf(" - %s\n", elements[item].c_str());
			}
		}
		printf("\n");
	}
	return 0;
}

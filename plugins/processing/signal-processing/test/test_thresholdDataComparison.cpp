/*
 * \author Alison Cellard / Inria
 * \date 30.08.2013
 */

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <complex>
#include <fstream>
#include <iostream>


using namespace OpenViBE;
using namespace std;



/* Method to read a file a store it in a buffer */
boolean fileToBuffer(const CString sFileName, std::vector<float64>& output )
{
	std::vector<string> l_vsData; // To store string data

	output.clear();

	float64 l_f64data = 0;

	std::ifstream l_file; // To read file
	stringstream ss; // to convert string to float64

	l_file.open(sFileName);

	if ( ! l_file.is_open() ) 
	{
		cout << "Error : could not open file " << sFileName << "\n";
		return false;
	}
	
	bool firstLineRead = false;

	while(l_file.good())
	{
		string l_sData;
		if(!firstLineRead)
		{
			getline(l_file, l_sData);
			firstLineRead = true;
		}
		else
			getline(l_file, l_sData,';');
		l_vsData.push_back(l_sData);
	}

	l_file.close();

	//use the first line to identify how many columns there are
	int columns=0;
	size_t found = l_vsData[0].find(';');
	while(found!=std::string::npos)
	{
		columns++;
		found = l_vsData[0].find(';', found+1);
	}
	cout << "Found "<<columns<<" columns\n";

	for(uint32 i = 1; i<l_vsData.size();i++)
	{

		ss.str(l_vsData[i]);
		ss >> l_f64data;
		ss.clear();
		output.push_back(l_f64data);
	}

	return true;
}


/* Method to compare two files */
boolean compareBuffers (const std::vector<float64>& rInVector, const std::vector<float64>& rOutVector, const float64 f64threshold)
{
	float64 l_f64Error;
	uint32 l_ui32NbError = 0;
	bool hadError = false;
	std::vector<float64> difference;

	// Compare size
	if(rInVector.size() == 0 || rOutVector.size()==0)
	{
		hadError = true;
		cout<<"Error : vector size = 0, no data to compare \n"<<endl;
	}

	if(rInVector.size() == rOutVector.size())
	{
		int cpt = 0;
		l_f64Error = 0;

		for(uint32 i=0;i<rOutVector.size();i++)
		{
			if(abs(rInVector[i]-rOutVector[i]) > f64threshold)
		     {
		         hadError = true;
		         l_ui32NbError = l_ui32NbError + 1;

		         if(abs(rInVector[i]-rOutVector[i]) > l_f64Error)
		         {
		        	 l_f64Error = abs(rInVector[i]-rOutVector[i]);
		        	 cpt = i;
		         }
//		         break;
		      }
			difference.push_back(rInVector[i]-rOutVector[i]);
		}

		if(hadError)
		{
			cout<<"Comparison failed, "<<l_ui32NbError<<" data differ, the largest difference is "<<l_f64Error<< " at value ["<<cpt<<"]\n"<<endl;
			cout << rInVector[cpt] << " ins of " << rOutVector[cpt] << endl;
		}

	}

	else
	{
		hadError = true;
		cout<<"Error : Files have different size, check input data"<<endl;
	}

	double mean=0;
	double var=0;
	for(unsigned int i=0; i<difference.size(); i++)
	{
		mean+=difference[i];
		var+=difference[i]*difference[i];
	}
	mean/=(double)difference.size();
	var/=(double)difference.size();
	var-=mean*mean;
	cout << "mean " << mean << " var " << var << endl;

	return !hadError;
}


// validation Test take in the algorithm output file, the expected output file (reference) and a tolerance threshold
boolean validationTest(const CString sOutputFile, const CString sRefOutputFile, const float64 f64threshold)
{
	boolean l_bIsTestPassed = true;

	std::vector<float64> l_vOutputVector;
	std::vector<float64> l_vRefOutputVector;

	l_bIsTestPassed &= fileToBuffer(sOutputFile, l_vOutputVector);
	if(!l_bIsTestPassed) return false;
	l_bIsTestPassed &= fileToBuffer(sRefOutputFile, l_vRefOutputVector);
	if(!l_bIsTestPassed) return false;
	l_bIsTestPassed &= compareBuffers(l_vOutputVector, l_vRefOutputVector,f64threshold);

	return l_bIsTestPassed;
}



int main(int argc, char *argv[])
{
	if(argc!=4) 
	{
		cout<<"Usage: " << argv[0] << " <data> <reference> <threshold>\n";
		return -1;
	}

	const float64 l_f64precisionTolerance = strtod(argv[3], NULL);

	boolean l_bValidationTestPassed = false;

	l_bValidationTestPassed = validationTest(argv[1],argv[2],l_f64precisionTolerance);


	if(!l_bValidationTestPassed)
	{
		cout<<"Algorithm failed validation test \n"<<endl;
		return 1;
	}
	else
	{
		cout<<"test passed\n"<<endl;
		return 0;
	}

}

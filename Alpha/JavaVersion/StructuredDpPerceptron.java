import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Collections; //sorting
import java.util.ArrayList;
import java.util.Random;

/*
The StructuredPerceptron framework has a universal and elegant structure, with
a swappable inference method hidden inside. This structured percetron implementation is very
different, however, as the x inputs are variable length sequences, and the method uses dynamic
programming a la genomic alignment methods. One notable difference is that the weights are passed
into the phi() function, and thus there's an interdependence between phi and the weights. 

Primary public methods:
	Train()
	Test()
*/
public class StructuredDpPerceptron
{
	double[] _weights;
	double _alpha;
	InferenceAlgorithm _inferenceAlgorithm;

	public StructuredDpPerceptron(int weightDimension, double alpha, String vocabPath)
	{
		_alpha = alpha;
		_weights = new double[weightDimension];
		_initWeights();
		_inferenceAlgorithm = new InferenceAlgorithm(weightDimension, vocabPath);
	}

	private void _initWeights()
	{
		//_setRandomWeights();
		_zeroWeights();
	}

	private void _zeroWeights()
	{
		for(int i = 0; i < _weights.length; i++){
			_weights[i] = -1.0;
		}
	}

	//not so important for linear perceptron as for neural nets, since the weights are updated directly by x vals
	private void _setRandomWeights()
	{
		Random r = new Random();
		//initialize _weights to random small values
		for(int i = 0; i < _weights.length; i++){
			_weights[i] = -1.0 * r.nextDouble() / 100;
		}
	}

	//Utility for tracking rank metrics, such as tracking the ranking of y-star in the results, over iterations.
	private int _getRank(ArrayList<SearchResult> results, String word)
	{
		int i = 0;
		int rank = -1;
		boolean found = false;
		
		for(i = 0, found = false; i < results.size() && !found; i++){
			if(results.get(i).GetWord().equals(word)){
				rank = i;
				found = true; 
			}
		}
		
		return rank;
	}

	/*
	Runs canonical structured perceptron training. On completion, _weights will have been trained
	and can be used for prediction.
	
	@D: A dataset composed of structure input/output pairs (x,y), both sequences/vectors. This is different from the canonical
	structured prediction format, as x in this domain is a sequence x of (x,y) point coordinates (sensor readings), and
	y is the intended output word w, which is a sequence of (x,y) points representing the letter coordinate sequence of some word.
	See Dataset class.
	*/
	public void Train(StructuredDataset D, int maxIterations)
	{
		int i;
		double[] phiHat;
		double[] phiStar;
		double avgRanking;
		boolean updateOccurred, isConverged;
		SearchResult yHat;
		ArrayList<SearchResult> inferences;

		System.out.println("Running training...");	
		//loop until convergence: no update occurs in inner loop or maxIterations reached
		//This loop attempts to maximize the ranking of y_star, hence performance is measured as such, over iterations.
		for(i = 0, isConverged = false; i < maxIterations && !isConverged; i++){
			updateOccurred = false;
			//the canonical structured perceptron loop
			avgRanking = 0.0;
			for(StructuredExample d : D.GetTrainingExamples()){
				_printWeights();
				inferences = _inferenceAlgorithm.Infer(d.XSequence, _weights);
				yHat = inferences.get(0);
				avgRanking += _getRank(inferences, d.Word);
				//check for update
				if(!yHat.GetWord().equals(d.Word)){
					System.out.println("Incorrectly predicted "+yHat.GetWord()+" for "+d.Word);
					//note the uniqueness of this framework, as the weights are passed in to derive Phi()
					phiHat =  _inferenceAlgorithm.Phi(d.XSequence, yHat.GetWord(), _weights);
					phiStar = _inferenceAlgorithm.Phi(d.XSequence, d.Word, _weights);
					//update weights
					_updateWeights(phiHat, phiStar);
					updateOccurred = true;
				}
				else{
					System.out.println("Correctly predicted "+yHat.GetWord()+" for "+d.Word);
				}
			}
			//track the average ranking of the correct prediction, to observe whether or not the rank is reduced as desired
			avgRanking /= (double)D.GetSize();
			
			System.out.println("Iteration "+Integer.toString(i)+" avg y-star rank: "+Double.toString(avgRanking));			
			_printWeights();
			
			//model converged if no predictions were incorrect for above loop
			isConverged = !updateOccurred;
		}
		System.out.println("Training complete.");
		_printWeights();
	}

	private void _printWeights()
	{
		System.out.print("Weights:\n");
		for(int i = 0; i < _weights.length; i++){
			System.out.print(Double.toString(_weights[i])+" ");
		}
		System.out.print("\n");
	}

	/*
	Implements the canonical weight update of the structured perceptron, when predicted yHat != yStar.
		w += alpha * (phi(x,yStar) - phi(x,yHat))
	
	@xSeq: The variable length x-sequence of sensor input (x,y) coordinates
	@yHat: The string representation of the predicted y output
	@yStar: The string representation of the target y output
	
	Note that both yHat and yStar don't have much meaning in this domain, except through _inferenceAlgorithm, 
	which contains the knowledge for mapping these labels into coordinate sequences, as well as the phi function.
	*/
	private void _updateWeights(double[] phiHat, double[] phiStar)
	{
		//ArrayList<double> phiHat, phiStar;
		
		//get phiHat
		//phiHat  = _inferenceAlgorithm.Phi(xSeq, yHat);
		//get phi
		//phiStar = _inferenceAlgorithm.Phi(xSeq, yStar);
		//weight update
		//System.out.println(phiHat.length);
		
		//System.out.println(phiHat[0]+" "+phiHat[1]+" "+phiHat[2]+" "+phiHat[3]+" "+phiHat[4]+" "+phiHat[5]);
		//System.out.println(phiStar[0]+" "+phiStar[1]+" "+phiStar[2]+" "+phiStar[3]+" "+phiStar[4]+" "+phiStar[5]);
		for(int i = 0; i < phiHat.length; i++){
			_weights[i] = _weights[i] + _alpha * (phiStar[i] - phiHat[i]);
		}
		
		//_normalizeWeights();
	}
	
	//converts weight vector to unit length
	private void _normalizeWeights()
	{
		double sum = 0.0;
		
		for(int i = 0; i < _weights.length; i++){
			sum += _weights[i];
		}
		
		for(int i = 0; i < _weights.length; i++){
			_weights[i] /= sum;
		}
	}

	public static void main(String[] args)
	{
		//String vocabPath = "./resources/languageModels/vocab.txt";
		String vocabPath = "./resources/languageModels/testTruncVocab.txt";
		String keyMapFile = "./resources/ui/keyMap.txt";
		StructuredDataset trainingData = new StructuredDataset(keyMapFile);
		StructuredDpPerceptron dpPerceptron = new StructuredDpPerceptron(6, 0.000001, vocabPath);
		
		String[] trainingFiles = new String[]{"./resources/testing/structuredData/word1.txt",
											"./resources/testing/structuredData/word2.txt",
											"./resources/testing/structuredData/word3.txt",
											"./resources/testing/structuredData/word4.txt",
											"./resources/testing/structuredData/word5.txt",
											"./resources/testing/structuredData/word6.txt",
											"./resources/testing/structuredData/word7.txt",
											"./resources/testing/structuredData/word8.txt",
											"./resources/testing/structuredData/word9.txt",
											"./resources/testing/structuredData/word10.txt",
											"./resources/testing/structuredData/word11.txt"};

		trainingData.BuildTrainingData(trainingFiles);

		dpPerceptron.Train(trainingData,60);
		//twitch.TestLinearDP(wordFile);
	}
}




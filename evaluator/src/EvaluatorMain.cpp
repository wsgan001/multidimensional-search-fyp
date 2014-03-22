#include <iostream>
#include <fstream>
#include "CommandLineArguments.h"
#include "IndexStructureFactory.h"
#include "DatasetFileLoader.h"
#include "TestOperationLoader.h"
#include "Evaluator.h"

using namespace mdsearch;

void writeResultsToFile(const std::string& filename,
	const OperationListTimings& timings,
	const CommandLineArguments& args,
	const std::vector<TestOperationList>& testOperationLists)
{
	std::ofstream file(filename.c_str());
	// Write structures used
	file << "===STRUCTURES===\n";
	const std::vector<CommandLineArguments::IndexStructureSpecification> structureSpecs = args.indexStructures();
	for (unsigned int i = 0; (i < structureSpecs.size()); i++)
	{
		const CommandLineArguments::IndexStructureSpecification& spec = structureSpecs[i];
		file << "Structure (" << i << "):\n\tType: " << spec.type << "\n\tDimensionality: " << spec.numDimensions << "\n\t";
		file << "Arguments: ";
		if (spec.arguments.size() > 0)
		{
			for (unsigned int j = 0; (j < spec.arguments.size() -1); j++)
				file << spec.arguments[j] << " ";
			file << spec.arguments[spec.arguments.size() -1];
		}
		file << "\n";
	}
	file << "\n";
	// Write test operation list information
	file << "===OPERATION LISTS===\n";
	for (unsigned int i = 0; (i < testOperationLists.size()); i++)
	{
		file << "Test Operation List (" << i << "):\n";
		file << "\tFilename: " << args.testOperationFilenames()[i] << "\n";
		file << "\tNumber of Operations: " << testOperationLists[i].size() << "\n";
	}
	file << "\n";
	// Write timing report
	file << "===TIMINGS===\n";
	file << generateTimingReport(timings);
	file.close();
}

int main(int argc, char* argv[])
{
	// Parse command line arguments
	CommandLineArguments args(argc, argv);
	// If the arguments are not valid, exit program with failure code 
	if (!args.isValid())
	{
		std::cout << "Invalid command line arguments (use -h or --help for accepted arguments)." << std::endl;
		return 1;
	}

	// Construct specified index structures
	if (args.isVerbose())
		std::cout << "Loading index structures" << std::endl;	
	std::vector<IndexStructure*> structures;

	IndexStructureFactory structureFactory;
	const std::vector<CommandLineArguments::IndexStructureSpecification> structureSpecs = args.indexStructures();
	for (unsigned int i = 0; (i < structureSpecs.size()); i++)
	{
		// Use specification to load index structure
		const CommandLineArguments::IndexStructureSpecification& spec = structureSpecs[i];
		IndexStructure* structure = structureFactory.constructIndexStructure(
			spec.type, spec.numDimensions, spec.arguments);
		// If structure could not be constructed, display error message and exist program
		if (!structure)
		{
			std::cout << "Could not load structure \"" << spec.type << "\" with arguments: [ ";
			for (unsigned int j = 0; (j < spec.arguments.size()); j++)
			{
				std::cout << "\"" << spec.arguments[j] << "\" ";
			}
			std::cout << "]" << std::endl;
			return 1;
		}
		structures.push_back(structure);
	}

	// Load specified datasets
	if (args.isVerbose())
		std::cout << "Loading datasets" << std::endl;	
	std::vector<PointList> datasets;

	DatasetFileLoader datasetLoader;
	const StringList& datasetFilenames = args.datasetFilenames();
	for (unsigned int i = 0; (i < datasetFilenames.size()); i++)
	{
		datasets.push_back( datasetLoader.load(datasetFilenames[i]) );
	}

	// Load specified test operations
	std::vector<TestOperationList> testOperationLists;
	if (args.isVerbose())
		std::cout << "Loading test operations" << std::endl;

	TestOperationLoader testOpLoader;
	const StringList& testOperationFilenames = args.testOperationFilenames();
	for (unsigned int i = 0; (i < testOperationFilenames.size()); i++)
	{
		testOperationLists.push_back(testOpLoader.loadFromFile(
			testOperationFilenames[i]) );
	}

	// Create evaluator object and fill it with loaded structures
	Evaluator evaluator(structures, args.testRunsToPerform(),
		args.profileCPU(), args.profileHeap(), args.isVerbose());
	// Run evaluation on loaded datasets and operation lists
	OperationListTimings timings = evaluator.timePerformance(testOperationLists);
	std::cout << generateTimingReport(timings) << std::endl;
	// Write timing results to file if an output filename was given
	if (!args.resultFilename().empty())
	{
		writeResultsToFile(args.resultFilename(), timings, args, testOperationLists);
	}

	return 0;
}
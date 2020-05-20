#include <map>

#include <vtkSmartPointer.h>
#include <vtkXMLReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLCompositeDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkDataSetReader.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkStructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkCellTypes.h>
#include <vtkResampleToImage.h>
#include <vtkImageExport.h>
#include <vtkMetaImageWriter.h>
#include <vtksys/SystemTools.hxx>

#include <itkVTKImageToImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>

template<class TReader>
vtkDataSet *ReadAnXMLFile(const char*fileName);

template<typename PixelType>
int exportImage(vtkImageData* imageData)
{
  vtkPointData* imgPtData = imageData->GetPointData();
  vtkDataArray* imgPtDataArray = imgPtData->GetScalars();
  int imgPtDataDim = imgPtDataArray->GetNumberOfComponents();
  std::cout << "You selected " << imgPtDataArray->GetName() << std::endl;
  if (imgPtDataDim != 1)
  {
    std::cerr << "Sorry, no handling has been defined for multi-dimensional "
      "data yet" << std::endl;
      std::cout << "No error output????";
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 3; // vtkImageData always has dimension 3
  using ImageType = itk::Image<PixelType, Dimension>;
  using FilterType = itk::VTKImageToImageFilter<ImageType>;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(imageData);

  try
  {
    filter->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "Error when converting VTK image to ITK image: " << e << std::endl;
    return EXIT_FAILURE;
  }

  ImageType::ConstPointer myitkImage = filter->GetOutput();

  // Export to NRRD
  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  //io->SetNrrdVectorType(nrrdKindList);
  //io->SetFileType(itk::ImageIOBase::ASCII);

  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();
  //nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput(myitkImage);
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName("out.nrrd");
  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "Error when writing NRRD: " << e << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
  if (argc < 2)
    std::cerr << "Usage: " << argv[0] << " XMLFile1 XMLFile2 ..." << std::endl;

  // Process each file on the command line
  int f = 1;
  while (f < argc)
  {
    vtkDataSet *dataSet;
    std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(argv[f]);
    // Dispatch based on the file extension
    if (extension == ".vtu")
      dataSet = ReadAnXMLFile<vtkXMLUnstructuredGridReader> (argv[f]);
    else if (extension == ".vtp")
      dataSet = ReadAnXMLFile<vtkXMLPolyDataReader> (argv[f]);
    else if (extension == ".vts")
      dataSet = ReadAnXMLFile<vtkXMLStructuredGridReader> (argv[f]);
    else if (extension == ".vtr")
      dataSet = ReadAnXMLFile<vtkXMLRectilinearGridReader> (argv[f]);
    else if (extension == ".vti")
      dataSet = ReadAnXMLFile<vtkXMLImageDataReader> (argv[f]);
    else if (extension == ".vtk")
      dataSet = ReadAnXMLFile<vtkDataSetReader> (argv[f]);
    else
    {
      std::cerr << argv[0] << " Unknown extension: " << extension << std::endl;
      return EXIT_FAILURE;
    }

    int numberOfCells = dataSet->GetNumberOfCells();
    int numberOfPoints = dataSet->GetNumberOfPoints();

    // Generate a report
    std::cout << "------------------------" << std::endl;
    std::cout << argv[f] << std::endl
         << " contains a " << std::endl
         << dataSet->GetClassName()
         <<  " that has " << numberOfCells << " cells"
         << " and " << numberOfPoints << " points." << std::endl;
    f++;
    typedef std::map<int,int> CellContainer;
    CellContainer cellMap;
    for (int i = 0; i < numberOfCells; i++)
      cellMap[dataSet->GetCellType(i)]++;

    CellContainer::const_iterator it = cellMap.begin();
    while (it != cellMap.end())
    {
      std::cout << "\tCell type "
           << vtkCellTypes::GetClassNameFromTypeId(it->first)
           << " occurs " << it->second << " times." << std::endl;
      ++it;
    }

    // Now check for point data
    vtkPointData *pd = dataSet->GetPointData();
    if (pd)
    {
      std::cout << " contains point data with "
           << pd->GetNumberOfArrays()
           << " arrays." << std::endl;
      for (int i = 0; i < pd->GetNumberOfArrays(); i++)
        std::cout << "\tArray " << i
             << " is named "
             << (pd->GetArrayName(i) ? pd->GetArrayName(i) : "NULL")
             << std::endl;
    }
    // Now check for cell data
    vtkCellData *cd = dataSet->GetCellData();
    if (cd)
    {
      std::cout << " contains cell data with "
           << cd->GetNumberOfArrays()
           << " arrays." << std::endl;
      for (int i = 0; i < cd->GetNumberOfArrays(); i++)
      {
        vtkDataArray* cda = cd->GetArray(i);
        std::string typeStr;
        switch (cda->GetDataType()) {
          case VTK_CHAR:
            typeStr = "char";
            break;
          case VTK_INT:
            typeStr = "int";
            break;
          case VTK_FLOAT:
            typeStr = "float";
            break;
          case VTK_DOUBLE:
            typeStr = "double";
            break;
          default:
            typeStr = std::to_string(cda->GetDataType());
            break;
        }
        std::cout << "\tArray " << i
             << " is named "
             << (cda->GetName() ? cda->GetName() : "NULL")
             << " and has "
             << cda->GetNumberOfComponents()
             << " component(s) of type "
             << typeStr
             << std::endl;
      }
    }
    // Now check for field data
    if (dataSet->GetFieldData())
    {
      std::cout << " contains field data with "
           << dataSet->GetFieldData()->GetNumberOfArrays()
           << " arrays." << std::endl;
      for (int i = 0; i < dataSet->GetFieldData()->GetNumberOfArrays(); i++)
      {
        std::cout << "\tArray " << i
             << " is named " << dataSet->GetFieldData()->GetArray(i)->GetName()
             << std::endl;
      }
    }

    // Sample dataset on a uniform grid in parallel
    vtkSmartPointer<vtkResampleToImage> resampleFilter =
      vtkSmartPointer<vtkResampleToImage>::New();
    resampleFilter->SetInputDataObject(dataSet);
    resampleFilter->SetSamplingDimensions(64, 64, 64);
    resampleFilter->Update();
    vtkImageData* imageData = resampleFilter->GetOutput();

    int dims[3];
    imageData->GetDimensions(dims);
    std::cout << "Output dimensions: "
      << dims[0] << ", "
      << dims[1] << ", "
      << dims[2] << std::endl;

    // set the scalars as the chosen field
    std::cout << "Enter the index of the cell data array you which to extract:"
      << std::endl;
    int imgPtDataInd;
    std::cin >> imgPtDataInd;

    vtkPointData* imgPtData = imageData->GetPointData();
    // FIXME: imgPtDataInd doesn't match the index in the cell data iterator
    vtkDataArray* imgPtDataArray = imgPtData->GetArray(imgPtDataInd);
    imgPtData->SetScalars(imgPtDataArray);

    int res = EXIT_FAILURE;
    switch (imgPtDataArray->GetDataType()) {
      case VTK_CHAR:
        exportImage<char>(imageData);
        break;
      case VTK_INT:
        exportImage<int>(imageData);
        break;
      case VTK_FLOAT:
        exportImage<float>(imageData);
        break;
      case VTK_DOUBLE:
        exportImage<double>(imageData);
        break;
      default:
        std::cerr << "Sorry, export has not yet been implemented for this type."
          << std::endl;
        break;
    }
    if (res != EXIT_SUCCESS) {
      dataSet->Delete();
      return res;
    }

#if 0
    // Write image data to an MHA file
    vtkSmartPointer<vtkMetaImageWriter> writer =
      vtkSmartPointer<vtkMetaImageWriter>::New();
    writer->SetInputData(imageData);
    writer->SetCompression(false); 
    writer->SetFileName("out.mha");
    writer->Write();
    continue;

    // Get the size of the image data
    int dims[3];
    imageData->GetDimensions(dims);

    // Create the c-style image to convert the VTK image to
    unsigned char *cImage = new unsigned char[dims[0]*dims[1]*dims[2]];

    vtkSmartPointer<vtkImageExport> exporter =
      vtkSmartPointer<vtkImageExport>::New();
    exporter->SetInputData(imageData);
    exporter->ImageLowerLeftOn();
    exporter->Update();
    exporter->Export(cImage);

    // Output the raw c-style image
    for(int row = 0; row < 3; ++row)
    {
      for(int col = 0; col < 3; ++col)
      {
        for(int sli = 0; sli < 3; ++sli)
          std::cout << static_cast<int>(
            cImage[(sli * dims[1] + col) * dims[0] + row]) << " ";
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }
    delete [] cImage;
#endif

    dataSet->Delete();

  }
  return EXIT_SUCCESS;
}

template<class TReader>
vtkDataSet *ReadAnXMLFile(const char*fileName)
{
  vtkSmartPointer<TReader> reader =
    vtkSmartPointer<TReader>::New();
  reader->SetFileName(fileName);
  reader->Update();
  reader->GetOutput()->Register(reader);
  return vtkDataSet::SafeDownCast(reader->GetOutput());
}
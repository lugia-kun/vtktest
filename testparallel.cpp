
#include <iostream>
#include <sstream>
#include <mpi.h>

#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkProcessGroup.h"
#include "vtkMPIController.h"
#include "vtkMPICommunicator.h"
#include "vtkProgrammableFilter.h"
#include "vtkRTAnalyticSource.h"
#include "vtkImageData.h"
#include "vtkContourFilter.h"
#include "vtkExtentTranslator.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkXMLPRectilinearGridWriter.h"
#include "vtkXMLPRectilinearGridReader.h"
#include "vtkInformation.h"

void PfExecute(void *arg)
{
  vtkProgrammableFilter *pf;
  pf = reinterpret_cast<vtkProgrammableFilter *>(arg);

  vtkInformation *info = pf->GetOutputInformation(0);
  vtkExtentTranslator *et = vtkExtentTranslator::New();

  if (pf->GetInput()->IsA("vtkDataSet")) {
    et->SetWholeExtent(info->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
    et->SetPiece(info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
    et->SetNumberOfPieces(info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
    et->PieceToExtent();
  }

  vtkDataObject *output = pf->GetOutput();
  vtkDataObject *input = pf->GetInput();

  output->ShallowCopy(input);
  if (pf->GetInput()->IsA("vtkDataSet")) {
    output->Crop(et->GetExtent());
  }

  et->Delete();
}

void ParMain(vtkMultiProcessController *controller, void *arg)
{
  int nranks = controller->GetNumberOfProcesses();
  int rank   = controller->GetLocalProcessId();

  vtkProgrammableFilter *pf = vtkProgrammableFilter::New();
  pf->SetExecuteMethod(PfExecute, pf);

  vtkRTAnalyticSource *s = vtkRTAnalyticSource::New();
  s->Update();

  vtkImageData *input = s->GetOutput();

  vtkRectilinearGrid *rg = vtkRectilinearGrid::New();
  rg->SetExtent(input->GetExtent());
  int *dims = input->GetDimensions();
  double *spacing = input->GetSpacing();

  vtkFloatArray *x = vtkFloatArray::New();
  x->SetNumberOfTuples(dims[0]);
  for (vtkIdType i = 0; i < dims[0]; ++i) {
    x->SetValue(i, spacing[0] * i);
  }

  vtkFloatArray *y = vtkFloatArray::New();
  y->SetNumberOfTuples(dims[1]);
  for (vtkIdType i = 0; i < dims[1]; ++i) {
    y->SetValue(i, spacing[1] * i);
  }

  vtkFloatArray *z = vtkFloatArray::New();
  z->SetNumberOfTuples(dims[2]);
  for (vtkIdType i = 0; i < dims[2]; ++i) {
    z->SetValue(i, spacing[2] * i);
  }

  rg->SetXCoordinates(x);
  rg->SetYCoordinates(y);
  rg->SetZCoordinates(z);

  rg->GetPointData()->ShallowCopy(input->GetPointData());

  pf->SetInputData(rg);
  pf->Update();
  pf->GetOutputInformation(0)->Print(std::cout);

  vtkSmartPointer<vtkXMLPRectilinearGridWriter> writer =
    vtkSmartPointer<vtkXMLPRectilinearGridWriter>::New();

  int npieces = 16;

  //writer->SetController(controller);
  writer->SetInputConnection(pf->GetOutputPort());
  writer->SetNumberOfPieces(npieces);
  int pperrank = npieces / nranks;
  int start = pperrank * rank;
  int end = start + pperrank - 1;
  writer->SetStartPiece(start);
  writer->SetEndPiece(end);
  writer->SetFileName("rttest.pvtr");
  writer->SetUseSubdirectory(true);
  writer->Write();

  pf->Delete();
  rg->Delete();
  s->Delete();
  x->Delete();
  y->Delete();
  z->Delete();

  controller->Barrier();

  vtkSmartPointer<vtkXMLPRectilinearGridReader> reader =
    vtkSmartPointer<vtkXMLPRectilinearGridReader>::New();

  vtkContourFilter *cf = vtkContourFilter::New();

  reader->SetFileName("rttest.pvtr");
  reader->UpdateInformation();

  cf->SetValue(0, 130.0);
  cf->SetComputeNormals(0);
  cf->SetComputeGradients(0);
  cf->SetInputConnection(reader->GetOutputPort());
  cf->UpdateInformation();

  vtkInformation *info = cf->GetOutputInformation(0);
  info->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(), nranks);
  info->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(), rank);
  cf->Update();

  cf->Delete();
}

int VtkMain(int argc, char **argv)
{
  vtkMPIController *controller = vtkMPIController::New();
  controller->Initialize(&argc, &argv, 1);
  int nprc = controller->GetNumberOfProcesses();
  int rank = controller->GetLocalProcessId();

  controller->SetSingleMethod(ParMain, 0);
  controller->SingleMethodExecute();

  controller->Finalize(1);
  controller->Delete();

  return 0;
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  VtkMain(argc, argv);

  MPI_Finalize();
  return 0;
}

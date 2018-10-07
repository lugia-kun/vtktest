
#include <iostream>
#include <sstream>
#include <mpi.h>

#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include "vtkCellData.h"
#include "vtkRectilinearGrid.h"
#include "vtkXMLPMultiBlockDataWriter.h"
#include "vtkProcessGroup.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMPIController.h"
#include "vtkMPICommunicator.h"

const static double x[5] = {
  0.0, 0.5, 1.0, 1.5, 2.0
};

const static double y[4] = {
  0.0, 0.3, 0.7, 1.0,
};

const static double z[5] = {
  0.0, 0.2, 0.4, 0.6, 0.8,
};

const static double d[2*3*4] = {
  0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
  7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0,
  14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0,
  21.0, 22.0, 23.0,
};

void ParMain(vtkMultiProcessController *controller, void *arg)
{
  vtkFloatArray *xCoords = vtkFloatArray::New();
  vtkFloatArray *yCoords = vtkFloatArray::New();
  vtkFloatArray *zCoords = vtkFloatArray::New();
  vtkFloatArray *dArray = vtkFloatArray::New();
  vtkMultiBlockDataSet *mSet = vtkMultiBlockDataSet::New();
  vtkRectilinearGrid *rGrid = vtkRectilinearGrid::New();
  vtkSmartPointer<vtkXMLPMultiBlockDataWriter> writer =
    vtkSmartPointer<vtkXMLPMultiBlockDataWriter>::New();

  int rank = controller->GetLocalProcessId();
  int nprc = controller->GetNumberOfProcesses();

  for (int i = 0; i < 3; ++i) xCoords->InsertNextValue(x[i + rank * 2]);
  for (int i = 0; i < 4; ++i) yCoords->InsertNextValue(y[i]);
  for (int i = 0; i < 5; ++i) zCoords->InsertNextValue(z[i]);
  for (int i = 0; i < 24; ++i) dArray->InsertNextValue(d[i] + rank * 24);

  xCoords->SetName("x coords");
  yCoords->SetName("y coords");
  zCoords->SetName("z coords");
  dArray->SetName("t");

  // rGrid->SetDimensions(3, 4, 5);
  rGrid->SetExtent(2 * rank, (2 * (rank + 1)), 0, 3, 0, 4);
  rGrid->SetXCoordinates(xCoords);
  rGrid->SetYCoordinates(yCoords);
  rGrid->SetZCoordinates(zCoords);
  rGrid->GetCellData()->AddArray(dArray);

  mSet->SetNumberOfBlocks(nprc);
  mSet->SetBlock(rank, rGrid);

  writer->SetController(controller);
  writer->SetFileName("test.vtm");
  writer->SetInputDataObject(mSet);
  writer->SetCompressorTypeToNone();
  writer->SetDataModeToAppended();
  writer->EncodeAppendedDataOff();
  writer->Write();

  rGrid->Delete();
  xCoords->Delete();
  yCoords->Delete();
  zCoords->Delete();
  dArray->Delete();
}

int VtkMain(int argc, char **argv)
{
  vtkMPIController *master = vtkMPIController::New();
  master->Initialize(&argc, &argv, 1);
  int nprc = master->GetNumberOfProcesses();
  int rank = master->GetLocalProcessId();

  vtkProcessGroup *group = vtkProcessGroup::New();
  group->Initialize(master);

  for (int i = 2; i < nprc; ++i) {
    int pid = group->GetProcessId(i);
    group->RemoveProcessId(pid);
  }
  vtkMPIController *controller = master->CreateSubController(group);
  group->Delete();

  if (!controller) {
    master->Finalize(1);
    master->Delete();

    return 0;
  }

  std::stringstream sstr;
  controller->Print(sstr);
  nprc = controller->GetNumberOfProcesses();
  rank = controller->GetLocalProcessId();
  if (rank == 0) {
    std::cout << sstr.str();
    for (int i = 1; i < nprc; ++i) {
      int n;
      controller->Receive(&n, 1, i, i);
      char *m = new char[n + 1];
      controller->Receive(m, n + 1, i, i);
      std::cout << m;
      delete [] m;
    }
  } else {
    int n = sstr.str().length();
    controller->Send(&n, 1, 0, rank);
    controller->Send(sstr.str().c_str(), n + 1, 0, rank);
  }

  controller->SetSingleMethod(ParMain, 0);
  controller->SingleMethodExecute();

  controller->Finalize(1);
  master->Finalize(1);
  controller->Delete();
  master->Delete();

  return 0;
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);

  VtkMain(argc, argv);

  MPI_Finalize();
  return 0;
}

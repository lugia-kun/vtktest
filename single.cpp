
#include <iostream>
#include <sstream>
#include <mpi.h>

#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include "vtkCellData.h"
#include "vtkRectilinearGrid.h"
#include "vtkXMLRectilinearGridWriter.h"

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

int main(int argc, char **argv)
{
  vtkFloatArray *xCoords = vtkFloatArray::New();
  vtkFloatArray *yCoords = vtkFloatArray::New();
  vtkFloatArray *zCoords = vtkFloatArray::New();
  vtkFloatArray *dArray = vtkFloatArray::New();
  vtkRectilinearGrid *rGrid = vtkRectilinearGrid::New();
  vtkSmartPointer<vtkXMLRectilinearGridWriter> writer =
    vtkSmartPointer<vtkXMLRectilinearGridWriter>::New();

  for (int i = 0; i < 5; ++i) xCoords->InsertNextValue(x[i]);
  for (int i = 0; i < 4; ++i) yCoords->InsertNextValue(y[i]);
  for (int i = 0; i < 5; ++i) zCoords->InsertNextValue(z[i]);
  for (int i = 0; i < 24; ++i) dArray->InsertNextValue(d[i]);

  xCoords->SetName("x coords");
  yCoords->SetName("y coords");
  zCoords->SetName("z coords");
  dArray->SetName("t");

  rGrid->SetDimensions(3, 4, 5);
  rGrid->SetXCoordinates(xCoords);
  rGrid->SetYCoordinates(yCoords);
  rGrid->SetZCoordinates(zCoords);
  rGrid->GetCellData()->AddArray(dArray);

  writer->SetFileName("test-single.vtr");
  writer->SetInputData(rGrid);
  writer->SetCompressorTypeToNone();
  writer->SetDataModeToAppended();
  writer->EncodeAppendedDataOff();
  writer->Update();
  writer->Write();

  rGrid->Delete();
  xCoords->Delete();
  yCoords->Delete();
  zCoords->Delete();
  dArray->Delete();

  return 0;
}

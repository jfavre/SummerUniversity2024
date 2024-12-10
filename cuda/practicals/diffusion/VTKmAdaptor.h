#ifndef VTKmAdaptor_h
#define VTKmAdaptor_h

#include <vtkm/cont/ColorTable.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetBuilderUniform.h>
#include <vtkm/cont/Initialize.h>
#include <vtkm/io/VTKDataSetWriter.h>
#include <vtkm/cont/cuda/internal/CudaAllocator.h>
#include <vtkm/rendering/Actor.h>
#include <vtkm/rendering/CanvasRayTracer.h>
#include <vtkm/rendering/MapperRayTracer.h>
#include <vtkm/rendering/MapperPoint.h>
#include <vtkm/rendering/MapperWireframer.h>
#include <vtkm/rendering/Scene.h>
#include <vtkm/rendering/View3D.h>

namespace VTKmAdaptor
{
  vtkm::rendering::CanvasRayTracer  canvas(1080, 1080);
  vtkm::rendering::Scene            scene;
  vtkm::rendering::MapperPoint      mapper0;
  vtkm::rendering::MapperRayTracer  mapper1;
  vtkm::rendering::MapperWireframer mapper2;
  vtkm::cont::DataSet               dataSet;

  void Initialize(const double *temperature_data, const int nx, const int ny, const int offset=0)
{
  std::cout << "VTK-m::Initialize" << std::endl;

  vtkm::cont::DataSetBuilderUniform dataSetBuilder;
  vtkm::Id3 vdims(nx, ny, 1);
  vtkm::Vec3f Origin(0., 0., 0.);
  float spacing = 1.0/(nx-1.0);
  vtkm::Vec3f Spacing(spacing, spacing, spacing);
  dataSet = dataSetBuilder.Create(vdims, Origin, Spacing);

  auto dataArray = vtkm::cont::make_ArrayHandle(temperature_data, nx*ny, vtkm::CopyFlag::Off);
  dataSet.AddPointField("Density", dataArray);

  dataSet.PrintSummary(std::cout);

  vtkm::cont::ColorTable colorTable("viridis");
  colorTable.RescaleToRange(vtkm::Range(0.0, 1.0));
  vtkm::rendering::Actor actor(dataSet.GetCellSet(),
                               dataSet.GetCoordinateSystem(),
                               dataSet.GetField("Density"),
                               colorTable);

  // Adding Actor to the scene
  scene.AddActor(actor);
  
  mapper0.SetUsePoints();
  mapper0.SetRadius(0.02f);
  mapper0.UseVariableRadius(false);
  mapper0.SetRadiusDelta(0.05f);
}

void Execute(int it, int frequency)
{
  std::ostringstream fname;
  if(it % frequency == 0)
    {
    fname << "insitu." << it << ".png";

    vtkm::rendering::View3D view(scene, mapper0, canvas);
    auto camera = view.GetCamera();
    camera.SetModeTo3D();
    //camera.SetViewRange2D(vtkm::Range(-1.0, 2.0), vtkm::Range(-1.0, 2.0));
    /*
    camera.SetLookAt(vtkm::Vec3f(0.5, 0.5, 0.0));
    camera.SetPosition(vtkm::Vec3f(0.5, 0.5, 10.0));
    camera.SetViewUp(vtkm::Vec3f(0.0, 1.0, 0.0));
    */
    view.SetBackgroundColor(vtkm::rendering::Color(1.0f, 1.0f, 1.0f));
    view.SetForegroundColor(vtkm::rendering::Color(0.0f, 0.0f, 0.0f));
    view.Paint();
    view.SaveAs(fname.str());
    }
}

void Finalize()
{
  vtkm::io::VTKDataSetWriter writer("/dev/shm/dummy.vtk");
  writer.SetFileTypeToBinary();
  writer.WriteDataSet(dataSet);
}
}
#endif


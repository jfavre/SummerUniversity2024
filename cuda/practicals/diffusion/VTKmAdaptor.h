#ifndef VTKmAdaptor_h
#define VTKmAdaptor_h
#include <iomanip>
#include <vtkm/cont/CoordinateSystem.h>
#include <vtkm/cont/ColorTable.h>
#include <vtkm/cont/ArrayPortal.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/filter/contour/Contour.h>
#include <vtkm/filter/field_transform/PointTransform.h>
#include <vtkm/cont/DataSetBuilderUniform.h>
#include <vtkm/cont/Initialize.h>
#include <vtkm/io/VTKDataSetWriter.h>
#include <vtkm/cont/cuda/DeviceAdapterCuda.h>
#include <vtkm/cont/cuda/internal/CudaAllocator.h>
#include <vtkm/rendering/Actor.h>
#include <vtkm/rendering/CanvasRayTracer.h>
#include <vtkm/rendering/MapperRayTracer.h>
#include <vtkm/rendering/MapperPoint.h>
#include <vtkm/rendering/MapperWireframer.h>
#include <vtkm/rendering/Scene.h>
#include <vtkm/rendering/View3D.h>
#include <vtkm/rendering/View2D.h>
#include <vtkm/cont/FieldRangeCompute.h>
#include <vtkm/rendering/WorldAnnotator.h>

namespace VTKmAdaptor
{
  vtkm::rendering::CanvasRayTracer  canvas(1024,1024);
  vtkm::rendering::ColorBarAnnotation     m_color_bar_annotation;

  vtkm::rendering::WorldAnnotator   *annotate;
  vtkm::rendering::MapperPoint      mapper0;
  vtkm::rendering::MapperRayTracer  mapperRaytracer;
  vtkm::rendering::MapperWireframer mapperWireframe;
  vtkm::cont::DataSet               dataSet;
  vtkm::cont::ColorTable colorTable("viridis");
  std::vector<vtkm::rendering::Color> White {vtkm::rendering::Color::white};

void Initialize(const int nx, const int ny, const int rank=0)
{
#if defined VTKm_ENABLE_CUDA
  std::cout << "[using VTK-m cuda support]" << std::endl;
#endif
  float spacing = 1.0/(nx-1.0);
  vtkm::Id3 dimensions(nx, ny,1);
  vtkm::Vec3f origin(0., rank * spacing * (ny-1), 0.);
  std::cout << "rank " << rank << " has y-offset "<< rank * spacing * (ny-1)<< std::endl;
  vtkm::Vec3f Spacing(spacing, spacing, spacing);
  vtkm::cont::ArrayHandleUniformPointCoordinates coords(dimensions, origin, Spacing);
  vtkm::cont::CoordinateSystem cs("coords", coords);
  dataSet.AddCoordinateSystem(cs);

  vtkm::cont::CellSetStructured<2> cellSet;
  cellSet.SetPointDimensions(vtkm::Id2(dimensions[0], dimensions[1]));
  dataSet.SetCellSet(cellSet);
  annotate = canvas.CreateWorldAnnotator();
  m_color_bar_annotation.SetColorTable(colorTable);
  m_color_bar_annotation.SetFieldName("Temperature");
}

void Execute(int it, double *temperature_data, const int size, const int rank=0)
{
  vtkm::rendering::Scene scene;
  try{
  auto dataArray = vtkm::cont::make_ArrayHandle(temperature_data, size, vtkm::CopyFlag::Off);
  dataSet.AddPointField("Temperature", dataArray);
  vtkm::cont::ArrayHandle<vtkm::Range> drange = vtkm::cont::FieldRangeCompute(dataSet, "Temperature");
  auto range = drange.ReadPortal().Get(0);
  std::cout << "range(Temperature) = " << range << std::endl;

  colorTable.RescaleToRange(range);
  vtkm::rendering::Actor mesh_actor(dataSet.GetCellSet(),
                               dataSet.GetCoordinateSystem(),
                               dataSet.GetField("Temperature"),
                               colorTable);

  vtkm::filter::contour::Contour contour;
  contour.SetGenerateNormals(false);
  contour.SetMergeDuplicatePoints(true);
  contour.SetIsoValues({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9});
  contour.SetActiveField("Temperature");
  contour.SetFieldsToPass(vtkm::filter::FieldSelection::Mode::All);
  vtkm::cont::DataSet isolines = contour.Execute(dataSet);
  
  vtkm::filter::field_transform::PointTransform xform;
  xform.SetTranslation(vtkm::Vec3f(0.0f, 0.0f, 0.001f));
 
  vtkm::cont::DataSet isolines2 = xform.Execute(isolines);

  vtkm::rendering::Actor isolines_actor(isolines2.GetCellSet(),
                               isolines2.GetCoordinateSystem(),
                               isolines2.GetField("Temperature"),
                               vtkm::rendering::Color(1.0f, 1.0f, 1.0f, 1.0f));

  scene.AddActor(mesh_actor);
  scene.AddActor(isolines_actor);
  
  m_color_bar_annotation.SetRange(vtkm::Range(0.0, 1.0), 5);
  canvas.Clear();
  vtkm::rendering::View3D view(scene, mapperWireframe, canvas);
  auto camera = view.GetCamera();

  camera.SetLookAt(vtkm::Vec3f(0.5, 0.5, 0.0));
  camera.SetPosition(vtkm::Vec3f(0.5, 0.5, 10.0));
  camera.SetViewUp(vtkm::Vec3f(0.0, 1.0, 0.0));

  view.SetBackgroundColor(vtkm::rendering::Color(0.0f, 0.0f, 0.0f));
  view.SetForegroundColor(vtkm::rendering::Color(1.0f, 1.0f, 1.0f));

  view.Paint();
  view.RenderScreenAnnotations();
  m_color_bar_annotation.Render(camera, *annotate, canvas);

  std::ostringstream fname;
  fname << "./datasets/Temperature-vtkm." << std::setfill('0') << std::setw(6) << it 
        << "." << std::setfill('0') << std::setw(2) << rank <<".png";
  view.SaveAs(fname.str());
  }
  catch (const vtkm::cont::Error& error)
  {
    std::cerr << "VTK-m error occurred!: " << error.GetMessage() << std::endl;
  }
}

void Finalize()
{
  delete annotate;
  // crash when using CUDA
  
  //
  //vtkm::io::VTKDataSetWriter writer("/dev/shm/dummy.vtk");
  //writer.SetFileTypeToBinary();
  //writer.WriteDataSet(dataSet);
  //
}
}
#endif


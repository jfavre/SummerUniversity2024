#ifndef ViskoresAdaptor_h
#define ViskoresAdaptor_h
#include <iomanip>
#include <viskores/cont/CoordinateSystem.h>
#include <viskores/cont/ColorTable.h>
#include <viskores/cont/ArrayPortal.h>
#include <viskores/cont/DataSet.h>
#include <viskores/filter/contour/Contour.h>
#include <viskores/filter/field_transform/PointTransform.h>
#include <viskores/cont/DataSetBuilderUniform.h>
#include <viskores/cont/Initialize.h>
#include <viskores/io/VTKDataSetWriter.h>
#include <viskores/cont/cuda/DeviceAdapterCuda.h>
#include <viskores/cont/cuda/internal/CudaAllocator.h>
#include <viskores/rendering/Actor.h>
#include <viskores/rendering/CanvasRayTracer.h>
#include <viskores/rendering/MapperRayTracer.h>
#include <viskores/rendering/MapperPoint.h>
#include <viskores/rendering/MapperWireframer.h>
#include <viskores/rendering/Scene.h>
#include <viskores/rendering/View3D.h>
#include <viskores/cont/FieldRangeCompute.h>
#include <viskores/rendering/WorldAnnotator.h>

namespace ViskoresAdaptor
{
  viskores::rendering::CanvasRayTracer  canvas(1024,1024);
  viskores::rendering::ColorBarAnnotation     m_color_bar_annotation;

  viskores::rendering::WorldAnnotator   *annotate;

  viskores::rendering::MapperPoint      mapper0;
  viskores::rendering::MapperRayTracer  mapperRaytracer;
  viskores::rendering::MapperWireframer mapperWireframe;
  viskores::cont::DataSet               dataSet;
  viskores::cont::ColorTable colorTable("viridis");
  std::vector<viskores::rendering::Color> White {viskores::rendering::Color::white};
    
void Initialize(const int nx, const int ny, const int rank=0)
{
#if defined Viskores_ENABLE_CUDA
  std::cout << "[using Viskores cuda support]" << std::endl;
#endif
  float spacing = 1.0/(nx-1.0);
  viskores::Id3 dimensions(nx, ny,1);
  viskores::Vec3f origin(0., rank * spacing * (ny-1), 0.);
  std::cout << "rank " << rank << " has y-offset "<< rank * spacing * (ny-1)<< std::endl;
  viskores::Vec3f Spacing(spacing, spacing, spacing);
  viskores::cont::ArrayHandleUniformPointCoordinates coords(dimensions, origin, Spacing);
  viskores::cont::CoordinateSystem cs("coords", coords);
  dataSet.AddCoordinateSystem(cs);
  
  viskores::cont::CellSetStructured<2> cellSet;
  cellSet.SetPointDimensions(viskores::Id2(dimensions[0], dimensions[1]));
  dataSet.SetCellSet(cellSet);
  annotate = canvas.CreateWorldAnnotator();
  m_color_bar_annotation.SetColorTable(colorTable);
  m_color_bar_annotation.SetFieldName("Temperature");
}

void Execute(int it, double *temperature_data, const int size, const int rank=0)
{
  viskores::rendering::Scene scene;

  // seems like the reference to the data gets "lost", so we must redo it at each step
  auto dataArray = viskores::cont::make_ArrayHandle(temperature_data, size, viskores::CopyFlag::Off);
  dataSet.AddPointField("Temperature", dataArray);
  viskores::cont::ArrayHandle<viskores::Range> drange = viskores::cont::FieldRangeCompute(dataSet, "Temperature");
  auto range = drange.ReadPortal().Get(0);
  std::cout << "range(Temperature) = " << range << std::endl;

  colorTable.RescaleToRange(range);
  viskores::rendering::Actor mesh_actor(dataSet, "Temperature", colorTable);

  viskores::filter::contour::Contour contour;
  contour.SetGenerateNormals(false);
  contour.SetMergeDuplicatePoints(true);
  contour.SetIsoValues({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9});
  contour.SetActiveField("Temperature");
  contour.SetFieldsToPass(viskores::filter::FieldSelection::Mode::All);
  viskores::cont::DataSet isolines = contour.Execute(dataSet);

// need to offset the isolines by a small amount in the direction of the eye-point
  viskores::filter::field_transform::PointTransform xform;
  xform.SetTranslation(viskores::Vec3f(0.0f, 0.0f, 0.001f));
 
  viskores::cont::DataSet isolines2 = xform.Execute(isolines);

  viskores::rendering::Actor isolines_actor(isolines2, "Temperature",
                               viskores::rendering::Color(1.0f, 1.0f, 1.0f, 1.0f));

  scene.AddActor(mesh_actor);
  scene.AddActor(isolines_actor);
  
  m_color_bar_annotation.SetRange(viskores::Range(0.0, 1.0), 5);

  canvas.Clear();
  viskores::rendering::View3D view(scene, mapperWireframe, canvas);
  auto camera = view.GetCamera();

  camera.SetLookAt(viskores::Vec3f(0.5, 0.5, 0.0));
  camera.SetPosition(viskores::Vec3f(0.5, 0.5, 10.0));
  camera.SetViewUp(viskores::Vec3f(0.0, 1.0, 0.0));

  view.SetBackgroundColor(viskores::rendering::Color(0.0f, 0.0f, 0.0f));
  view.SetForegroundColor(viskores::rendering::Color(1.0f, 1.0f, 1.0f));

  view.Paint();
  view.RenderScreenAnnotations();
  m_color_bar_annotation.Render(camera, *annotate, canvas);

  std::ostringstream fname;
  fname << "./datasets/Temperature-Viskores." << std::setfill('0') << std::setw(6) << it 
        << "." << std::setfill('0') << std::setw(2) << rank <<".png";
  view.SaveAs(fname.str());
}

void Finalize()
{
  delete annotate;
  // crash when using CUDA
  
  //
  //viskores::io::VTKDataSetWriter writer("/dev/shm/dummy.vtk");
  //writer.SetFileTypeToBinary();
  //writer.WriteDataSet(dataSet);
  //
}
}
#endif


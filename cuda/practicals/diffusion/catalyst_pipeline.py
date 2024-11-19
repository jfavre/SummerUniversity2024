# script-version: 2.0
# Catalyst state generated using paraview version 5.13.1
import paraview
paraview.compatibility.major = 5
paraview.compatibility.minor = 13

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1024,1024]
renderView1.InteractionMode = '2D'
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [0.44736841320991516, 0.44736841320991516, 0.0]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [0.44736842438578606, 0.44736842438578606, 2.9973684433847665]
renderView1.CameraFocalPoint = [0.44736842438578606, 0.44736842438578606, 0.0]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 0.6326744773387929
renderView1.LegendGrid = 'Legend Grid Actor'
renderView1.PolarGrid = 'Polar Grid Actor'
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

reader = TrivialProducer(registrationName='grid')

contour1 = Contour(registrationName='Contour1', Input=reader)
contour1.ContourBy = ['POINTS', 'temperature']
contour1.ComputeNormals = 0
contour1.Isosurfaces = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
contour1.PointMergeMethod = 'Uniform Binning'

readerDisplay = Show(reader, renderView1, 'UniformGridRepresentation')

# get color transfer function/color map for 'temperature'
temperatureLUT = GetColorTransferFunction('temperature')
temperatureLUT.ScalarRangeInitialized = 1.0

# get opacity transfer function/opacity map for 'temperature'
temperaturePWF = GetOpacityTransferFunction('temperature')
temperaturePWF.ScalarRangeInitialized = 1

# trace defaults for the display properties.
readerDisplay.Representation = 'Surface With Edges'
readerDisplay.Representation = 'Surface'
readerDisplay.ColorArrayName = ['POINTS', 'temperature']
readerDisplay.LookupTable = temperatureLUT
readerDisplay.OSPRayScaleArray = 'temperature'
readerDisplay.OSPRayScaleFunction = 'Piecewise Function'
readerDisplay.Assembly = 'Hierarchy'
readerDisplay.SelectedBlockSelectors = ['']
readerDisplay.SelectOrientationVectors = 'None'
readerDisplay.ScaleFactor = 0.08947368487715722
readerDisplay.SelectScaleArray = 'None'
readerDisplay.SetScaleArray = ['POINTS', 'temperature']
readerDisplay.ScaleTransferFunction = 'Piecewise Function'
readerDisplay.OpacityArray = ['POINTS', 'temperature']
readerDisplay.OpacityTransferFunction = 'Piecewise Function'
readerDisplay.DataAxesGrid = 'Grid Axes Representation'
readerDisplay.PolarAxes = 'Polar Axes Representation'
readerDisplay.ScalarOpacityUnitDistance = 0.1913863855411576
readerDisplay.ScalarOpacityFunction = temperaturePWF

# init the 'Plane' selected for 'SliceFunction'
readerDisplay.SliceFunction.Origin = [0.44736842438578606, 0.44736842438578606, 0.0]

# show data from contour1
contour1Display = Show(contour1, renderView1, 'GeometryRepresentation')

# trace defaults for the display properties.
contour1Display.Representation = 'Surface'
contour1Display.ColorArrayName = ['POINTS', '']

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
contour1Display.ScaleTransferFunction.Points = [0.25, 0.0, 0.5, 0.0, 0.75, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
contour1Display.OpacityTransferFunction.Points = [0.25, 0.0, 0.5, 0.0, 0.75, 1.0, 0.5, 0.0]

# setup the color legend parameters for each legend in this view

# get color legend/bar for temperatureLUT in view renderView1
temperatureLUTColorBar = GetScalarBar(temperatureLUT, renderView1)
temperatureLUTColorBar.Title = 'temperature'
temperatureLUTColorBar.ComponentTitle = ''

# set color bar visibility
temperatureLUTColorBar.Visibility = 1

# show color legend
readerDisplay.SetScalarBarVisibility(renderView1, True)

pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
pNG1.Trigger = 'TimeStep'
pNG1.Trigger.Frequency = 1
pNG1.Writer.FileName = 'view-{timestep:06d}{camera}.png'
pNG1.Writer.ImageResolution = [1024,1024]
pNG1.Writer.Format = 'PNG'

# Catalyst options
from paraview import catalyst
options = catalyst.Options()
options.GlobalTrigger = 'Time Step'
options.CatalystLiveTrigger = 'Time Step'

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)

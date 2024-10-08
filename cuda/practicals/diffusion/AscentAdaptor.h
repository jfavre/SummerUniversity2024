#ifndef AscentAdaptor_h
#define AscentAdaptor_h

#include <ascent/ascent.hpp>
#include "conduit_blueprint.hpp"

namespace AscentAdaptor
{
  ascent::Ascent ascent;
  conduit::Node mesh;
  conduit::Node actions;

  void Initialize(const double *x1, const int nx, const int ny, const int offset=0)
{ 
  std::cout << "AscentInitialize.........................................\n";

  // default name for ghost field is used
  // ascent_options["ghost_field_name"].append() = "ascent_ghosts";

  mesh["coordsets/coords/dims/i"].set(nx);
  mesh["coordsets/coords/dims/j"].set(ny);
  // do not specify the 3rd dimension with a dim of 1, a z_origin, and a z_spacing

  float spacing = 1.0/(nx+1.0);
 
  mesh["coordsets/coords/origin/x"].set(0.0);
  mesh["coordsets/coords/origin/y"].set(offset * spacing * (ny-1));
  //std::cout << "Oy = "<< offset * spacing * (ny-1) << "\n";
  mesh["coordsets/coords/type"].set("uniform");

  mesh["coordsets/coords/spacing/dx"].set(spacing);
  mesh["coordsets/coords/spacing/dy"].set(spacing);
  
  // add topology.
  mesh["topologies/mesh/type"].set("uniform");
  mesh["topologies/mesh/coordset"].set("coords");

  // temperature is vertex-data.
  mesh["fields/temperature/association"].set("vertex");
  mesh["fields/temperature/type"].set("scalar");
  mesh["fields/temperature/topology"].set("mesh");
  mesh["fields/temperature/volume_dependent"].set("false");
  mesh["fields/temperature/values"].set_external((double *)x1, nx * ny);

  conduit::Node verify_info;
  if (!conduit::blueprint::mesh::verify(mesh, verify_info))
    {
    // verify failed, print error message
    CONDUIT_INFO("blueprint verify failed!" + verify_info.to_json());
    }
  //else CONDUIT_INFO("blueprint verify success!");

  conduit::Node &add_action0 = actions.append();
  add_action0["action"] = "add_pipelines";
  conduit::Node &pipelines = add_action0["pipelines"];
  pipelines["pl1/f1/type"] = "clip_with_field";
  pipelines["pl1/f1/params/field"] = "temperature";
  pipelines["pl1/f1/params/invert"] = "false";
  pipelines["pl1/f1/params/clip_value"] = .5;

  conduit::Node &add_action = actions.append();
  add_action["action"] = "add_scenes";
  conduit::Node &scenes       = add_action["scenes"];
  //scenes["s1/plots/p1/type"]  = "pseudocolor";
  //scenes["s1/plots/p1/field"] = "temperature";
  //scenes["view/plots/p2/type"]  = "mesh";
  // if showing the clipped_by_value scene, don't show plot p1 which would hide p2
  scenes["s1/plots/p2/type"]  = "pseudocolor";
  scenes["s1/plots/p2/field"]  = "temperature";
  scenes["s1/plots/p2/pipeline"]  = "pl1";
  scenes["s1/image_prefix"] = "temperature_%04d";
}

void Execute(int timestep, double dt)
{
  mesh["state/cycle"].set(timestep);
  mesh["state/time"].set(timestep * dt);

  ascent.publish(mesh);
  ascent.execute(actions);
}

void Finalize()
{
#ifdef IO_FROM_DEVICE_NOT_WORKING
  ascent.publish(mesh);
  conduit::Node action;
  conduit::Node &add_extr = action.append();
  add_extr["action"] = "add_extracts";
  conduit::Node &extracts = add_extr["extracts"];
  extracts["e1/type"] = "relay";
  extracts["e1/params/path"] = "/dev/shm/heatmesh";
  extracts["e1/params/protocol"] = "hdf5";
  ascent.execute(actions);
#endif
  ascent.close();
  std::cout << "AscentFinalize.........................................\n";
}
}
#endif

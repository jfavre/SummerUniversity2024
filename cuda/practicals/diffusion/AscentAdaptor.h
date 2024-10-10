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
  mesh["fields/temperature/values"].set_external((double *)x1, nx * ny); // x1 is either on the host or on the device

  conduit::Node verify_info;
  if (!conduit::blueprint::mesh::verify(mesh, verify_info))
    {
    // verify failed, print error message
    CONDUIT_INFO("blueprint verify failed!" + verify_info.to_json());
    }
  //else CONDUIT_INFO("blueprint verify success!");

  conduit::Node &add_action = actions.append();
  add_action["action"] = "add_scenes";
  conduit::Node &scenes       = add_action["scenes"];
  scenes["s1/plots/p1/type"]  = "pseudocolor";
  scenes["s1/plots/p1/field"] = "temperature";
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
#ifndef ASCENT_CUDA_ENABLED
  conduit::Node action;
  conduit::Node &add_action = action.append();
  
  add_action["action"] = "add_extracts";
  conduit::Node &extract = add_action["extracts"];
  extract["e1/type"] = "relay";
  extract["e1/params/path"] = "temperature_mesh";
  extract["e1/params/protocol"] = "hdf5";
  extract["e1/params/topologies"].append() = "mesh";
  ascent.publish(mesh);
  ascent.execute(action);
#endif
  ascent.close();
  std::cout << "AscentFinalize.........................................\n";
}
}
#endif

#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/jet.h>
#include <igl/planarize_quad_mesh.h>
#include <igl/quad_planarity.h>
#include <igl/readDMAT.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/opengl/glfw/Viewer.h>
#include <vector>
#include <cstdlib>
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/camera_view.h"


// Quad mesh generated from conjugate field
Eigen::MatrixXd VQC;
Eigen::MatrixXi FQC;
Eigen::MatrixXi FQCtri;
Eigen::MatrixXd PQC0, PQC1, PQC2, PQC3;

// Planarized quad mesh
Eigen::MatrixXd VQCplan;
Eigen::MatrixXi FQCtriplan;
Eigen::MatrixXd PQC0plan, PQC1plan, PQC2plan, PQC3plan;


// Scale for visualizing the fields
double global_scale; //TODO: not used


int main(int argc, char *argv[])
{
  using namespace Eigen;
  using namespace std;

  std::string baseDir = "../data/";

  std::string meshfilename = "deformed_plane.obj";
  std::string meshfilenameOut = "deformed_plane_planar.obj";

  // Concatenate the base directory path and the filename
  std::string meshPath = baseDir + meshfilename;
  std::string meshPathOut = baseDir + meshfilenameOut;
  std::cout << "Mesh path: " << meshPath << std::endl;

  // Read mesh
  igl::readOBJ(meshPath, VQC, FQC);

  // Convert it in a triangle mesh
  FQCtri.resize(2*FQC.rows(), 3);
  FQCtri <<  FQC.col(0),FQC.col(1),FQC.col(2),
             FQC.col(2),FQC.col(3),FQC.col(0);
  PQC0 = VQC(FQC.col(0).eval(), Eigen::all);
  PQC1 = VQC(FQC.col(1).eval(), Eigen::all);
  PQC2 = VQC(FQC.col(2).eval(), Eigen::all);
  PQC3 = VQC(FQC.col(3).eval(), Eigen::all);

  // Planarize it
  igl::planarize_quad_mesh(VQC, FQC, 100, 0.005, VQCplan);

  // Convert the planarized mesh to triangles
  PQC0plan = VQCplan(FQC.col(0).eval(), Eigen::all);
  PQC1plan = VQCplan(FQC.col(1).eval(), Eigen::all);
  PQC2plan = VQCplan(FQC.col(2).eval(), Eigen::all);
  PQC3plan = VQCplan(FQC.col(3).eval(), Eigen::all);


  // Read mesh
  igl::writeOBJ(meshPathOut, VQCplan, FQC);
  std::cout << "Mesh written to path: " << meshPathOut << std::endl;

  // Launch the viewer
  polyscope::init();

  //polyscope::view::setUpDir(polyscope::UpDir::ZUp);
  //polyscope::view::setFrontDir(polyscope::FrontDir::NegYFront);
  polyscope::options::groundPlaneMode = polyscope::GroundPlaneMode::None;

  // Define image dimensions
  double fov_v_deg = 90.0;
  int w = 640, h = 680;
  double aspect = static_cast<double>(w) / h;

  // Set up Polyscope camera parameters
  polyscope::CameraParameters params(
                polyscope::CameraIntrinsics::fromFoVDegVerticalAndAspect(fov_v_deg, aspect),
                polyscope::CameraExtrinsics::fromVectors(
                        glm::vec3{0., 0., 0.},
                        glm::vec3{0., 0., 1.},
                        glm::vec3{0., 1., 0.})
                );

  // Create Camera
  polyscope::CameraView* cam1 = polyscope::registerCameraView("Camera", params);

  auto planarMesh = polyscope::registerSurfaceMesh("non-planar mesh", VQC, FQC);
  auto nonplanarMesh = polyscope::registerSurfaceMesh("planar mesh", VQCplan, FQC);

  VectorXd nonplanarity;
  igl::quad_planarity(VQC, FQC, nonplanarity);
  MatrixXd Ct;
  igl::jet(nonplanarity, 0, 0.01, Ct);
  MatrixXd C(FQCtri.rows(),3);
  C << Ct, Ct;

  VectorXd planarity;
  igl::quad_planarity(VQCplan, FQC, planarity);
  MatrixXd Mt;
  igl::jet(planarity, 0, 0.01, Mt);
  MatrixXd M(FQCtri.rows(),3);
  M << Mt, Mt;

  // Register scalar quantities for visualization
  planarMesh->addFaceScalarQuantity("Planarity", planarity);
  nonplanarMesh->addFaceScalarQuantity("Non-Planarity", nonplanarity);

  polyscope::show();
}

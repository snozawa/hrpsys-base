#ifndef RATSMATRIX_H
#define RATSMATRIX_H
#include <hrpUtil/Eigen3d.h>
#include <iostream>

namespace rats
{
  inline bool eps_eq(const double a, const double b, const double eps = 0.001)
  {
    return fabs((a)-(b)) <= eps;
  };
  hrp::Vector3 matrix_log(const hrp::Matrix33& m);

  // matrix product using quaternion normalization
  void rotm3times (hrp::Matrix33& m12, const hrp::Matrix33& m1, const hrp::Matrix33& m2);
  void difference_rotation(hrp::Vector3& ret_dif_rot, const hrp::Matrix33& self_rot, const hrp::Matrix33& target_rot);

  struct coordinates {
    hrp::Vector3 pos;
    hrp::Matrix33 rot;
    coordinates() : pos(hrp::Vector3::Zero()), rot(hrp::Matrix33::Identity()) {};
    coordinates(const hrp::Vector3& p, const hrp::Matrix33& r) : pos(p), rot(r) {};
    coordinates(const hrp::Vector3& p) : pos(p), rot(hrp::Matrix33::Identity()) {};
    coordinates(const hrp::Matrix33& r) : pos(hrp::Vector3::Zero()), rot(r) {};
    coordinates(const coordinates& c) : pos(c.pos), rot(c.rot) {};
    virtual ~coordinates() {
    }
    coordinates& operator=(const coordinates& c) {
      if (this != &c) {
        pos = c.pos;
        rot = c.rot;
      }
      return *this;
    }
    //abc
    void inverse_transformation(coordinates& inv) const {
      inv.rot = rot.transpose();
      inv.pos = inv.rot*(-1 * pos);
    }
    void transformation(coordinates& tc, coordinates c, const std::string& wrt = ":local") const {
      tc = *this;
      inverse_transformation(tc);
      if (wrt == ":local") {
        tc.transform(c);
      } else if(wrt == ":world") {
        c.transform(tc);
        tc = c;
      } else {
        std::cerr << "**** invalid wrt! ****" << std::endl;
      }
    }
    void transform(const coordinates& c, const std::string& wrt = ":local") {
      if (wrt == ":local") {
        pos += rot * c.pos;
        // alias(rot) = rot * c.rot;
        hrp::Matrix33 rot_org(rot);
        rotm3times(rot, rot_org, c.rot);
      } else if (wrt == ":world") {
        hrp::Vector3 p(c.pos);
        hrp::Matrix33 r(c.rot);      
        p += r * pos; 
        rotm3times(r, c.rot, rot);      
        pos = p;
        rot = r;
      } else {
        std::cerr << "**** invalid wrt! ****" << std::endl;
      }
    }
  };

  void mid_coords(coordinates& mid_coords, const double p, const coordinates& c1, const coordinates& c2);
};
#endif /* RATSMATRIX_H */

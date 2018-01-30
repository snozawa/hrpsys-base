#ifndef OBJECTCONTACTTURNAROUNDDETECTORBASE_H
#define OBJECTCONTACTTURNAROUNDDETECTORBASE_H

#include "../TorqueFilter/IIRFilter.h"
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <cmath>
#include "hrpsys/util/Hrpsys.h"
#include <hrpUtil/Eigen3d.h>

class ObjectContactTurnaroundDetectorBase
{
 public:
    typedef enum {MODE_IDLE, MODE_STARTED, MODE_DETECTED, MODE_MAX_TIME} process_mode;
    typedef enum {TOTAL_FORCE, TOTAL_MOMENT, TOTAL_MOMENT2, GENERALIZED_WRENCH} detector_total_wrench;
 private:
    boost::shared_ptr<FirstOrderLowPassFilter<double> > wrench_filter;
    boost::shared_ptr<FirstOrderLowPassFilter<double> > dwrench_filter;
    boost::shared_ptr<FirstOrderLowPassFilter<double> > friction_coeff_wrench_filter;
    hrp::Vector3 axis, moment_center;
    hrp::dvector6 constraint_conversion_matrix1, constraint_conversion_matrix2;
    double prev_wrench, dt;
    double detect_ratio_thre, start_ratio_thre, ref_dwrench, max_time, current_time;
    double raw_wrench;
    size_t count;
    // detect_count_thre*dt and start_ratio_thre*dt are threshould for time.
    //   detect_count_thre*dt : Threshould for time [s] after the first object contact turnaround detection (Wait detect_time_thre [s] after first object contact turnaround detection).
    //   start_count_thre*dt  : Threshould for time [s] after the first starting detection (Wait start_time_thre [s] after first start detection).
    size_t detect_count_thre, start_count_thre;
    process_mode pmode;
    detector_total_wrench dtw;
    std::string print_str;
    bool is_filter_reset, is_hold_values;
 public:
    ObjectContactTurnaroundDetectorBase (const double _dt) : axis(-1*hrp::Vector3::UnitZ()), moment_center(hrp::Vector3::Zero()),
                                                             constraint_conversion_matrix1(hrp::dvector6::Zero()), constraint_conversion_matrix2(hrp::dvector6::Zero()),
                                                             prev_wrench(0.0), dt(_dt), detect_ratio_thre(0.01), start_ratio_thre(0.5),
                                                             count(0), detect_count_thre(5), start_count_thre(5), pmode(MODE_IDLE), dtw(TOTAL_FORCE), is_filter_reset(false), is_hold_values(false)
    {
        double default_cutoff_freq = 1; // [Hz]
        wrench_filter = boost::shared_ptr<FirstOrderLowPassFilter<double> >(new FirstOrderLowPassFilter<double>(default_cutoff_freq, _dt, 0));
        dwrench_filter = boost::shared_ptr<FirstOrderLowPassFilter<double> >(new FirstOrderLowPassFilter<double>(default_cutoff_freq, _dt, 0));
        friction_coeff_wrench_filter = boost::shared_ptr<FirstOrderLowPassFilter<double> >(new FirstOrderLowPassFilter<double>(default_cutoff_freq, _dt, 0));
    };
    ~ObjectContactTurnaroundDetectorBase () {};
    void startDetection (const double _ref_diff_wrench, const double _max_time)
    {
        ref_dwrench = _ref_diff_wrench/_max_time;
        max_time = _max_time;
        current_time = 0;
        count = 0;
        is_filter_reset = true;
        std::cerr << "[" << print_str << "] Start Object Turnaround Detection (ref_dwrench = " << ref_dwrench
                  << ", detect_thre = " << detect_ratio_thre * ref_dwrench << ", start_thre = " << start_ratio_thre * ref_dwrench << "), max_time = " << max_time << "[s]" << std::endl;
        pmode = MODE_IDLE;
    };
    hrp::Vector3 calcTotalForce (const std::vector<hrp::Vector3>& forces)
    {
        hrp::Vector3 tmpv = hrp::Vector3::Zero();
        for (size_t i = 0; i < forces.size(); i++) {
            tmpv += forces[i];
        }
        return tmpv;
    };
    hrp::Vector3 calcTotalMoment (const std::vector<hrp::Vector3>& forces, const std::vector<hrp::Vector3>& hposv)
    {
        hrp::Vector3 tmpv = hrp::Vector3::Zero();
        for (size_t i = 0; i < forces.size(); i++) {
            tmpv += (hposv[i]-moment_center).cross(forces[i]);
        }
        return tmpv;
    };
    hrp::Vector3 calcTotalMoment2 (const std::vector<hrp::Vector3>& forces, const std::vector<hrp::Vector3>& moments, const std::vector<hrp::Vector3>& hposv)
    {
        hrp::Vector3 tmpv = hrp::Vector3::Zero();
        for (size_t i = 0; i < forces.size(); i++) {
            tmpv += (hposv[i]-moment_center).cross(forces[i]) + moments[i];
        }
        return tmpv;
    };
    hrp::dvector6 calcTotalWrench (const std::vector<hrp::Vector3>& forces, const std::vector<hrp::Vector3>& moments, const std::vector<hrp::Vector3>& hposv)
    {
        // Total wrench around the origin
        hrp::Vector3 tmpf = hrp::Vector3::Zero();
        hrp::Vector3 tmpn = hrp::Vector3::Zero();
        for (size_t i = 0; i < forces.size(); i++) {
            tmpf += forces[i];
            tmpn += hposv[i].cross(forces[i]) + moments[i];
        }
        hrp::dvector6 ret;
        for (size_t i = 0; i < 3; i++) {
            ret(i) = tmpf(i);
            ret(i+3) = tmpn(i);
        }
        return ret;
    };
    bool checkDetection (const std::vector<hrp::Vector3>& forces,
                         const std::vector<hrp::Vector3>& moments,
                         const std::vector<hrp::Vector3>& hposv)
    {
        switch(dtw) {
        case TOTAL_FORCE:
            {
                hrp::Vector3 total_force = calcTotalForce(forces);
                checkDetection(axis.dot(total_force), total_force(2));
                break;
            }
        case TOTAL_MOMENT:
            {
                hrp::Vector3 total_moment = calcTotalMoment(forces, hposv);
                checkDetection(axis.dot(total_moment), 0.0);
            }
            break;
        case TOTAL_MOMENT2:
            {
                hrp::Vector3 total_moment = calcTotalMoment2(forces, moments, hposv);
                checkDetection(axis.dot(total_moment), 0.0);
            }
            break;
        case GENERALIZED_WRENCH:
            {
                hrp::dvector6 resultant_OR_wrench = calcTotalWrench(forces, moments, hposv);
                double phi1 = constraint_conversion_matrix1.dot(resultant_OR_wrench);
                double phi2 = constraint_conversion_matrix2.dot(resultant_OR_wrench);
                checkDetection(phi1, phi2);
            };
            break;
        default:
            break;
        };
    };
    bool checkDetection (const double wrench_value, const double friction_coeff_wrench_value)
    {
        if (is_filter_reset) {
          std::cerr << "[" << print_str << "] Object Turnaround Detection Reset Values. (wrench_value = " << wrench_value << ", friction_coeff_wrench_value = " << friction_coeff_wrench_value << ")" << std::endl;
          wrench_filter->reset(wrench_value);
          dwrench_filter->reset(0);
          friction_coeff_wrench_filter->reset(friction_coeff_wrench_value);
          is_filter_reset = false;
        }
        raw_wrench = wrench_value;
        double tmp_wr = wrench_filter->passFilter(wrench_value);
        double tmp_dwr = dwrench_filter->passFilter((tmp_wr-prev_wrench)/dt);
        friction_coeff_wrench_filter->passFilter(friction_coeff_wrench_value);
        prev_wrench = tmp_wr;
        // Checking of wrench profile turn around
        //   Sign of ref_dwrench and tmp_dwr shuold be same
        //   Supprot both ref_dwrench > 0 case and ref_dwrench < 0 case
        switch (pmode) {
        case MODE_IDLE:
            if ( (ref_dwrench > 0.0) ? (tmp_dwr > ref_dwrench*start_ratio_thre) : (tmp_dwr < ref_dwrench*start_ratio_thre) ) {
                count++;
                if (count > start_count_thre) {
                    pmode = MODE_STARTED;
                    count = 0;
                    std::cerr << "[" << print_str << "] Object Turnaround Detection Started. (" << start_count_thre*dt << "[s] after the first start detection)" << std::endl;
                }
            } else {
                /* count--; */
            }
            break;
        case MODE_STARTED:
            if ( (ref_dwrench > 0.0) ? (tmp_dwr < ref_dwrench*detect_ratio_thre) : (tmp_dwr > ref_dwrench*detect_ratio_thre) ) {
                count++;
                if (count > detect_count_thre) {
                    pmode = MODE_DETECTED;
                    std::cerr << "[" << print_str << "] Object Turnaround Detected (time = " << current_time << "[s], " << detect_count_thre*dt << "[s] after the first detection)" << std::endl;
                }
            } else {
                /* count--; */
            }
            //std::cerr << "[" << print_str << "] " << tmp_wr << " " << tmp_dwr << " " << count << std::endl;
            break;
        case MODE_DETECTED:
            break;
        case MODE_MAX_TIME:
            break;
        default:
            break;
        }
        if (max_time <= current_time && (pmode != MODE_DETECTED)) {
            if (pmode != MODE_MAX_TIME) std::cerr << "[" << print_str << "] Object Turnaround Detection max time reached." << std::endl;
            pmode = MODE_MAX_TIME;
        }
        current_time += dt;
        return isDetected();
    };
    bool isDetected () const { return (pmode == MODE_DETECTED); };
    process_mode getMode () const { return pmode; };
    void printParams () const
    {
        std::string tmpstr;
        switch (dtw) {
        case TOTAL_FORCE:
            tmpstr = "TOTAL_FORCE";break;
        case TOTAL_MOMENT:
            tmpstr = "TOTAL_MOMENT";break;
        case TOTAL_MOMENT2:
            tmpstr = "TOTAL_MOMENT2";break;
        case GENERALIZED_WRENCH:
            tmpstr = "GENERALIZED_WRENCH";break;
        default:
            tmpstr = "";break;
        }
        std::cerr << "[" << print_str << "]   ObjectContactTurnaroundDetectorBase params (" << tmpstr << ")" << std::endl;
        std::cerr << "[" << print_str << "]    wrench_cutoff_freq = " << wrench_filter->getCutOffFreq() << "[Hz], dwrench_cutoff_freq = " << dwrench_filter->getCutOffFreq() << "[Hz], friction_coeff_wrench_freq = " << friction_coeff_wrench_filter->getCutOffFreq() << "[Hz]" << std::endl;
        std::cerr << "[" << print_str << "]    detect_ratio_thre = " << detect_ratio_thre << ", start_ratio_thre = " << start_ratio_thre
                  << ", start_time_thre = " << start_count_thre*dt << "[s], detect_time_thre = " << detect_count_thre*dt << "[s]" << std::endl;
        std::cerr << "[" << print_str << "]    axis = [" << axis(0) << ", " << axis(1) << ", " << axis(2)
                  << "], moment_center = " << moment_center(0) << ", " << moment_center(1) << ", " << moment_center(2) << "][m]" << std::endl;
        std::cerr << "[" << print_str << "]    constraint_conversion_matrix1 = " << constraint_conversion_matrix1.format(Eigen::IOFormat(Eigen::StreamPrecision, 0, ", ", ", ", "", "", "[", "]"))
                  << ", constraint_conversion_matrix2 = " << constraint_conversion_matrix2.format(Eigen::IOFormat(Eigen::StreamPrecision, 0, ", ", ", ", "", "", "[", "]")) << std::endl;
        std::cerr << "[" << print_str << "]    is_hold_values = " << (is_hold_values?"true":"false") << std::endl;
    };
    void setPrintStr (const std::string& str) { print_str = str; };
    void setWrenchCutoffFreq (const double a) { wrench_filter->setCutOffFreq(a); };
    void setDwrenchCutoffFreq (const double a) { dwrench_filter->setCutOffFreq(a); };
    void setFrictionCoeffWrenchCutoffFreq (const double a) { friction_coeff_wrench_filter->setCutOffFreq(a); };
    void setDetectRatioThre (const double a) { detect_ratio_thre = a; };
    void setStartRatioThre (const double a) { start_ratio_thre = a; };
    void setDetectTimeThre (const double a) { detect_count_thre = round(a/dt); };
    void setStartTimeThre (const double a) { start_count_thre = round(a/dt); };
    void setAxis (const hrp::Vector3& a) { axis = a; };
    void setMomentCenter (const hrp::Vector3& a) { moment_center = a; };
    void setConstraintConversionMatrix1 (const hrp::dvector6& a) { constraint_conversion_matrix1 = a; };
    void setConstraintConversionMatrix2 (const hrp::dvector6& a) { constraint_conversion_matrix2 = a; };
    void setDetectorTotalWrench (const detector_total_wrench _dtw)
    {
        if (_dtw != dtw) {
          is_filter_reset = true;
        }
        dtw = _dtw;
    };
    void setIsHoldValues (const bool a) { is_hold_values = a; };
    double getWrenchCutoffFreq () const { return wrench_filter->getCutOffFreq(); };
    double getDwrenchCutoffFreq () const { return dwrench_filter->getCutOffFreq(); };
    double getFrictionCoeffWrenchCutoffFreq () const { return friction_coeff_wrench_filter->getCutOffFreq(); };
    double getDetectRatioThre () const { return detect_ratio_thre; };
    double getStartRatioThre () const { return start_ratio_thre; };
    double getDetectTimeThre () const { return detect_count_thre*dt; };
    double getStartTimeThre () const { return start_count_thre*dt; };
    hrp::Vector3 getAxis () const { return axis; };
    hrp::Vector3 getMomentCenter () const { return moment_center; };
    hrp::dvector6 getConstraintConversionMatrix1() const { return constraint_conversion_matrix1; };
    hrp::dvector6 getConstraintConversionMatrix2() const { return constraint_conversion_matrix2; };
    detector_total_wrench getDetectorTotalWrench () const { return dtw; };
    double getFilteredWrench () const { return wrench_filter->getCurrentValue(); };
    double getFilteredDwrench () const { return dwrench_filter->getCurrentValue(); };
    double getFilteredFrictionCoeffWrench () const { return friction_coeff_wrench_filter->getCurrentValue(); };
    double getRawWrench () const { return raw_wrench; };
    bool getIsHoldValues () const { return is_hold_values; };
};
#endif // OBJECTCONTACTTURNAROUNDDETECTORBASE_H

#include "solid.h"

namespace sdfibm {

// 辅助函数：将 quaternion 转换为 Euler angles（XYZ 顺序）
vector quaternionToEulerAnglesXYZ(const quaternion& q)
{
    // 从旋转张量提取 Euler angles
    tensor R = q.R();
    
    scalar roll  = atan2(R.zy(), R.zz());  // x-axis rotation
    scalar pitch = asin(-R.zx());         // y-axis rotation  
    scalar yaw   = atan2(R.xy(), R.xx());   // z-axis rotation
    
    return vector(roll, pitch, yaw);
}

std::ostream& operator<<(std::ostream& os, const Solid& s)
{
    vector v;
    v = s.getCenter();  os << v.x() << ' ' << v.y() << ' ' << v.z() << ' ';
    v = s.getVelocity();os << v.x() << ' ' << v.y() << ' ' << v.z() << ' ';
    v = s.getForce();   os << v.x() << ' ' << v.y() << ' ' << v.z() << ' ';

    // v = s.getOrientation().eulerAngles(quaternion::XYZ);
    v = quaternionToEulerAnglesXYZ(s.getOrientation());
                            os << v.x() << ' ' << v.y() << ' ' << v.z() << ' ';
    v = s.getOmega();   os << v.x() << ' ' << v.y() << ' ' << v.z() << ' ';
    v = s.getTorque();  os << v.x() << ' ' << v.y() << ' ' << v.z();
    return os;
}

void write2D(std::ostream& os, const Solid& s)
{
    vector v;
    v = s.getCenter();  os << v.x() << ' ' << v.y() << ' ';
    v = s.getVelocity();os << v.x() << ' ' << v.y() << ' ';
    v = s.getForce();   os << v.x() << ' ' << v.y() << ' ';
    // v = s.getOrientation().eulerAngles(quaternion::XYZ);
    v = quaternionToEulerAnglesXYZ(s.getOrientation());
                            os << v.z() << ' ';
    v = s.getOmega();   os << v.z() << ' ';
    v = s.getTorque();  os << v.z();
}

}
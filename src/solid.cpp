#include "solid.h"

namespace sdfibm {

// 辅助函数：将 quaternion 转换为 Euler angles（XYZ 顺序）
vector quaternionToEulerAnglesXYZ(const quaternion& q)
{
    // 从旋转张量提取 Euler angles
    tensor R = q.R();

    scalar roll, pitch, yaw;

    // pitch
    Foam::scalar sinPitch = Foam::max(-1.0, Foam::min(1.0, -R.zx()));
    pitch = Foam::asin(sinPitch);

    // roll and yaw
    // gimbal lock check
    if (Foam::mag(Foam::cos(pitch)) > 1e-6)
    {
        roll  = Foam::atan2(R.zy(), R.zz());  // x-axis rotation
        yaw   = Foam::atan2(R.xy(), R.xx());  // z-axis rotation
    }
    else
    {
        // 发生 Gimbal Lock：此时 X 轴和 Z 轴重合，只能计算两者的相对角度
        // 约定 yaw = 0，将所有旋转归结到 roll 上
        yaw   = 0.0;
        roll  = Foam::atan2(-R.yz(), R.yy()); 
    }
    
    // scalar roll  = atan2(R.zy(), R.zz());  // x-axis rotation
    // scalar pitch = asin(-R.zx());         // y-axis rotation  
    // scalar yaw   = atan2(R.xy(), R.xx());   // z-axis rotation
    
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
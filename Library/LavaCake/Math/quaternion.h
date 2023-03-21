#include "basics.h"
#pragma once

namespace LavaCake {
    template<typename T>
    struct i_t {
        T i =T(0);
    };

    template<typename T>
    struct j_t {
        T j =T(0);
    };


    template<typename T>
    struct k_t {
        T k =T(0);
    };

    template<typename T>
    struct quaternion{
        T qw = T(0);
        i_t<T> qx;
        j_t<T> qy;
        k_t<T> qz;


        /**
         * Create a quaternion from 4 components (a,b,c, and d)
         * the created quaternion is equal to a + i*b + j*c + k*d
        */
        quaternion(
        i_t<T> x,
        j_t<T> y,
        k_t<T> z,
        T w ){
            qw = w;
            qx = x;
            qy = y;
            qz = z;
        }

        
        quaternion(
        T angle, 
        vec<T,3>axis){
            qw = cos(angle/T(2.0));
            qx = i_t<T>{T(axis[0]*sin(angle/T(2.0)))};
            qy = j_t<T>{T(axis[1]*sin(angle/T(2.0)))};
            qz = k_t<T>{T(axis[2]*sin(angle/T(2.0)))};
        }

        quaternion(
        vec<T,3>dir1, 
        vec<T,3>dir2){
            auto d1 = normalize(dir1);
            auto d2 = normalize(dir2);
            
        }

        quaternion(){}
    };
}

//////////////////////////////////////////////
//I operator
//////////////////////////////////////////////
template<typename T>
LavaCake::i_t<T> operator *(const LavaCake::i_t<T>& a, const T& b){
    return LavaCake::i_t<T>{a.i * b};
}

template<typename T>
LavaCake::i_t<T> operator *(const T& a, const LavaCake::i_t<T>&  b){
    return LavaCake::i_t<T>{a * b.i};
}

template<typename T>
T operator *(const LavaCake::i_t<T>&  a, const LavaCake::i_t<T>&  b){
    return T(-a.i * b.i);
}

template<typename T>
LavaCake::i_t<T> operator +(const LavaCake::i_t<T>&  a, const LavaCake::i_t<T>&  b){
    return LavaCake::i_t<T>{a.i + b.i};
}

template<typename T>
LavaCake::i_t<T> operator -(const LavaCake::i_t<T>&  a, const LavaCake::i_t<T>&  b){
    return LavaCake::i_t<T>{a.i - b.i};
}

template<typename T>
LavaCake::i_t<T> operator -(const LavaCake::i_t<T>&  a){
    return LavaCake::i_t<T>{-a.i};
}

template<typename T>
bool operator ==(const LavaCake::i_t<T>&  a, const LavaCake::i_t<T>&  b){
    return a.i == b.i;
}


//////////////////////////////////////////////
//J operator
//////////////////////////////////////////////

template<typename T>
LavaCake::j_t<T> operator *(const LavaCake::j_t<T>& a, const T& b){
    return LavaCake::j_t<T>{a.j * b};
}

template<typename T>
LavaCake::j_t<T> operator *(const T& a, const LavaCake::j_t<T>&  b){
    return LavaCake::j_t<T>{a * b.j};
}

template<typename T>
T operator *(const LavaCake::j_t<T>&  a, const LavaCake::j_t<T>&  b){
    return T(-a.j * b.j);
}

template<typename T>
LavaCake::j_t<T> operator +(const LavaCake::j_t<T>&  a, const LavaCake::j_t<T>&  b){
    return LavaCake::j_t<T>{a.j + b.j};
}

template<typename T>
LavaCake::j_t<T> operator -(const LavaCake::j_t<T>&  a, const LavaCake::j_t<T>&  b){
    return LavaCake::j_t<T>{a.j - b.j};
}

template<typename T>
LavaCake::j_t<T> operator -(const LavaCake::j_t<T>&  a){
    return LavaCake::j_t<T>{-a.j};
}

template<typename T>
bool operator ==(const LavaCake::j_t<T>&  a, const LavaCake::j_t<T>&  b){
    return a.j == b.j;
}

//////////////////////////////////////////////
//K operator
//////////////////////////////////////////////

template<typename T>
LavaCake::k_t<T> operator *(const LavaCake::k_t<T>& a, const T& b){
    return LavaCake::k_t<T>{a.k * b};
}

template<typename T>
LavaCake::k_t<T> operator *(const T& a, const LavaCake::k_t<T>&  b){
    return LavaCake::k_t<T>{a * b.k};
}

template<typename T>
T operator *(const LavaCake::k_t<T>&  a, const LavaCake::k_t<T>&  b){
    return T(-a.k * b.k);
}

template<typename T>
LavaCake::k_t<T> operator +(const LavaCake::k_t<T>&  a, const LavaCake::k_t<T>&  b){
    return LavaCake::k_t<T>{a.k + b.k};
}

template<typename T>
LavaCake::k_t<T> operator -(const LavaCake::k_t<T>&  a, const LavaCake::k_t<T>&  b){
    return LavaCake::k_t<T>{a.k - b.k};
}

template<typename T>
LavaCake::k_t<T> operator -(const LavaCake::k_t<T>&  a){
    return LavaCake::k_t<T>{-a.k};
}

template<typename T>
bool operator ==(const LavaCake::k_t<T>&  a, const LavaCake::k_t<T>&  b){
    return a.k == b.k;
}

//////////////////////////////////////////////
//IJ operator
//////////////////////////////////////////////

template<typename T>
LavaCake::k_t<T> operator *(const LavaCake::i_t<T>&  a, const LavaCake::j_t<T>&  b){
    return LavaCake::k_t<T>{a.i * b.j};
}

template<typename T>
LavaCake::k_t<T> operator *(const LavaCake::j_t<T>&  a, const LavaCake::i_t<T>&  b){
    return LavaCake::k_t<T>{-a.j * b.i};
}


//////////////////////////////////////////////
//IK operator
//////////////////////////////////////////////

template<typename T>
LavaCake::j_t<T> operator *(const LavaCake::i_t<T>&  a, const LavaCake::k_t<T>&  b){
    return LavaCake::j_t<T>{-a.i * b.k};
}

template<typename T>
LavaCake::j_t<T> operator *(const LavaCake::k_t<T>&  a, const LavaCake::i_t<T>&  b){
    return LavaCake::j_t<T>{a.k * b.i};
}


//////////////////////////////////////////////
//JK operator
//////////////////////////////////////////////

template<typename T>
LavaCake::i_t<T> operator *(const LavaCake::j_t<T>&  a, const LavaCake::k_t<T>&  b){
    return LavaCake::i_t<T>{a.j * b.k};
}

template<typename T>
LavaCake::i_t<T> operator *(const LavaCake::k_t<T>&  a, const LavaCake::j_t<T>&  b){
    return LavaCake::i_t<T>{-a.k * b.j};
}

/////////////////////////////////////////////
// Quaternion operator
/////////////////////////////////////////////
template<typename T>
LavaCake::quaternion<T> operator +(const LavaCake::quaternion<T>& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a.qx + b.qx, a.qy + b.qy, a.qz + b.qz, a.qw +b.qw);
}

template<typename T>
LavaCake::quaternion<T> operator -(const LavaCake::quaternion<T>& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a.w -b.w, a.i - b.i, a.j - b.j, a.k - b.k);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::quaternion<T>& a, const T& b){
    return LavaCake::quaternion<T>(a.qx*b, a.qy*b, a.qz*b, a.qw*b);
}

template<typename T>
LavaCake::quaternion<T> operator *(const T& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a*b.qx, a*b.qy, a*b.qz, a*b.qw);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::quaternion<T>& a, const LavaCake::i_t<T>& b){
    return LavaCake::quaternion<T>(a.qx*b, a.qw*b, a.qz*b, a.qy*b);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::i_t<T>& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a*b.qw, a*b.qz, a*b.qy,a*b.qx);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::quaternion<T>& a, const LavaCake::j_t<T>& b){
    return LavaCake::quaternion<T>(a.qy*b, a.qz*b, a.qw*b, a.qx*b);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::j_t<T>& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a*b.qz, a*b.qw, a*b.qx, a*b.qy);
}

template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::quaternion<T>& a, const LavaCake::k_t<T>& b){
    return LavaCake::quaternion<T>(a.qy*b, a.qx*b, a.qw*b, a.qz*b);
}

template<typename T>
LavaCake::quaternion<T> operator *(const  LavaCake::k_t<T>& a, const LavaCake::quaternion<T>& b){
    return LavaCake::quaternion<T>(a*b.qy, a*b.qx, a*b.qw, a*b.qz);
}


template<typename T>
LavaCake::quaternion<T> operator *(const LavaCake::quaternion<T>& a, const LavaCake::quaternion<T>& b){
    return a.qw*b + a.qx*b + a.qy*b + a.qz*b;
}

template<typename T>
LavaCake::quaternion<T> inverse(const LavaCake::quaternion<T>& q){
    T sqrnorme = (q.qw*q.qw + q.qx.i * q.qx.i + q.qy.j * q.qy.j + q.qz.k * q.qz.k);
    return LavaCake::quaternion<T>(-q.qx,-q.qy,-q.qz,q.qw) * (T(1)/sqrnorme);
}

template<typename T>
LavaCake::quaternion<T> operator*(const LavaCake::quaternion<T>& q,LavaCake::vec<T,3>v){
    auto tmp = q*LavaCake::quaternion<T>({v[0]},{v[1]},{v[2]},T(0.0));
    return tmp;
    //return LavaCake::vec<T,3>({tmp.qx.i,tmp.qy.j, tmp.qz.k});
}

template<typename T>
LavaCake::quaternion<T> operator*(LavaCake::vec<T,3>v, const LavaCake::quaternion<T>& q){
    auto tmp = LavaCake::quaternion<T>({v[0]},{v[1]},{v[2]},T(0.0))*q;
    return tmp;
    //return LavaCake::vec<T,3>({tmp.qx.i,tmp.qy.j, tmp.qz.k});
}


template<typename T>
T dot(
    const LavaCake::quaternion<T>& q1, 
    const LavaCake::quaternion<T>& q2
){
    return q1.qx * q2.qx + q1.qy * q2.qy + q1.qz * q2.qz + q1.qw * q2.qw;
}

template<typename T>
LavaCake::quaternion<T> weightedSum(
    const LavaCake::quaternion<T>& q1,
    T w1,
    const LavaCake::quaternion<T>& q2,
    T w2
    ){
    
    
    auto dq1q2 = dot(q1,q2);


    if(dq1q2 != T(0)){

        auto z = (w1 - w2)*(w1 - w2) + T(4.0)*dq1q2*dq1q2;
        auto q = (w1 - w2 + z) * q1 + T(2.0)*w2*dq1q2 * q2;

        q = q  * (T(1)/sqrt(dot(q,q)));
        return q;
    }
    else{
        auto z = std::abs(w1 - w2);

        if(w1 >w2){
            auto q = (w1 - w2 + z) * q1;

            q = q  * (T(1)/sqrt(dot(q,q)));
            return q;
        }
        else {
            auto q = (w2 - w1 + z) * q2;

            q = q  * (T(1)/sqrt(dot(q,q)));
            return q;
        }
    }


}
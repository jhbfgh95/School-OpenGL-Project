#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT
};

//default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    //Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    //euler angles
    float Yaw;
    float Pitch;
    //Camera Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    //기본 생성자 - 벡터를 생성 매개변수로 받는
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH ):
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        upadteCameraVectors();
    }

    //생성자 오버로딩 - 스칼라 파라미터를 받는
    Camera(
        float posX, float posY, float posZ,
        float upX, float upY, float upZ, float yaw, float pitch) : 
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        upadteCameraVectors();
    }

    //Euler Angle과 LookAt Matrix를 계산한 ViewMatrix를 리턴함
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    //거속시 공식
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if(direction == FORWARD)
            Position += Front * velocity;
        if(direction == BACKWARD)
            Position -= Front * velocity;
        if(direction == RIGHT)
            Position += Right * velocity;
        if(direction == LEFT)
            Position -= Right * velocity;
    }

    //마우스 인풋을 받아온다. x와 y방향에서 원하는 offset value를 받아온다.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        //오일럭 각에서의 고질적 문제인, 짐벌락의 문제 해결을 위해 cosntrationpitch를 둔다.
        //블렌더 등의 모델링 툴처럼 쓰고 싶다면, false로 하고 추가 로직이 필요함.
        if(constrainPitch)
        {
            if(Pitch > 89.0f) Pitch = 89.0f;
            if(Pitch < -89.0f) Pitch = -89.0f;
        }

        //Euler Angle을 쓰는 Front, Right 벡터 업테이트
        upadteCameraVectors();
    }

    //마우스 스크롤 이벤트를 받는다. 수직 축 휠만 필요함 (그게 yoffset)
    void ProcessScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if(Zoom < 1.0f) Zoom = 1.0f;
        if(Zoom > 45.0f) Zoom = 45.0f;
    }
private:
    //카메라의 Euler Angle이 업데이트 되었을 때, Front Vector 계산해줌.
    void upadteCameraVectors()
    {
        //새 front vector 계산 (밑에는 공식이라고 생각)
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        //front vector가 바뀌었으니, right, up vector도 최신화
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
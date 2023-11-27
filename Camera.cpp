#include "pch.h"
#include "Camera.h"

//camera for our app simple directX application. While it performs some basic functionality its incomplete. 
//

Camera::Camera()
{
	//initalise values. 
	//Orientation and Position are how we control the camera. 
	m_orientation.x = -90.0f;		//rotation around x - pitch
	m_orientation.y = 0.0f;		//rotation around y - yaw
	m_orientation.z = 0.0f;		//rotation around z - roll	//we tend to not use roll a lot in first person

	m_position.x = 0.0f;		//camera position in space. 
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	//These variables are used for internal calculations and not set.  but we may want to queary what they 
	//externally at points
	m_lookat.x = 0.0f;		//Look target point
	m_lookat.y = 0.0f;
	m_lookat.z = 0.0f;

	m_forward.x = 0.0f;		//forward/look direction
	m_forward.y = 0.0f;
	m_forward.z = 0.0f;

	m_right.x = 0.0f;
	m_right.y = 0.0f;
	m_right.z = 0.0f;
	
	// Speed variables
	m_movespeed = 0.1f;
	m_boostspeed = 30;
	m_camRotRate = 0.5f;

	//active = false;
	//force update with initial values to generate other camera data correctly for first update. 
	Update();
}


Camera::~Camera()
{
}

void Camera::Update()
{
	if (active)
	{
		// DirectX::SimpleMath::Quaternion newRot;
		// newRot.x = sin(DirectX::XMConvertToRadians(m_orientation.y) * m_camRotRate/2);
		// newRot.y = sin(DirectX::XMConvertToRadians(m_orientation.z) * m_camRotRate/2);
		// newRot.z = cos(DirectX::XMConvertToRadians(m_orientation.y) * m_camRotRate/2);
		// newRot.w = 1;

		// DirectX::SimpleMath::Quaternion currentRot = DirectX::SimpleMath::Quaternion(m_forward.x, m_forward.y, m_forward.z,1);

		//rotation in yaw - using the paramateric equation of a circle
		// m_forward += DirectX::XMQuaternionSlerp(currentRot, newRot,0.3f);

		m_forward.x = sin(DirectX::XMConvertToRadians(m_orientation.y));
		m_forward.y = sin(DirectX::XMConvertToRadians(m_orientation.z));
		m_forward.z = cos(DirectX::XMConvertToRadians(m_orientation.y));
		m_forward.Normalize();

		//m_forward.x = sin((m_orientation.z) * 3.1415f / 180.0f);
		//m_forward.y = cos((m_orientation.z) * 3.1415f / 180.0f);
		//m_forward.Normalize();

		//create right vector from look Direction
		m_forward.Cross(DirectX::SimpleMath::Vector3::UnitY, m_right);

		//update lookat point
		m_lookat = m_position + m_forward;

		//apply camera vectors and create camera matrix
		m_cameraMatrix = (DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_lookat, DirectX::SimpleMath::Vector3::UnitY));
	}
}

DirectX::SimpleMath::Matrix Camera::getCameraMatrix()
{
	return m_cameraMatrix;
}

void Camera::setPosition(DirectX::SimpleMath::Vector3 newPosition)
{
	m_position = newPosition;
}

void Camera::setSmoothPosition(DirectX::SimpleMath::Vector3 newPosition)
{
	// Smooth movement using Lerp
	m_position = XMVectorLerp(m_position, newPosition, 0.01f);
}


DirectX::SimpleMath::Vector3 Camera::getPosition()
{
	return m_position;
}

DirectX::SimpleMath::Vector3 Camera::getForward()
{
	return m_forward;
}

DirectX::SimpleMath::Vector3 Camera::getRight()
{
	return m_right;
}

void Camera::setRotation(DirectX::SimpleMath::Vector3 newRotation)
{
	m_orientation = newRotation;
}

void Camera::setSmoothRotation(DirectX::SimpleMath::Vector3 newRotation)
{
	// Smooth rotation using Slerp
	m_orientation = XMQuaternionSlerp(m_orientation, newRotation,0.3f);
}


DirectX::SimpleMath::Vector3 Camera::getRotation()
{
	return m_orientation;
}

float Camera::getMoveSpeed()
{
	return m_movespeed;
}

float Camera::getBoostSpeed()
{
	return m_boostspeed;
}

float Camera::getRotationSpeed()
{
	return m_camRotRate;
}

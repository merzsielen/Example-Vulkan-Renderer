#ifndef GEOMETRY_H
#define GEOMETRY_H


/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Geometry.h																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#include <cmath>

namespace Untitled
{
	/*---------------------------------------------------------------------------------------------*/
	/* Vectors & Quaternions																	   */
	/*---------------------------------------------------------------------------------------------*/
	/*-------------------------------------------------------------------------*/
	/* 2-Dimensional Vectors												   */
	/*-------------------------------------------------------------------------*/
	struct Vec2
	{
		float x, y;

		/* Constructors -------------------------------------------*/
		Vec2()
		{
			this->x = 0;
			this->y = 0;
		}
		Vec2(float v)
		{
			this->x = v;
			this->y = v;
		}
		Vec2(float x, float y)
		{
			this->x = x;
			this->y = y;
		}
		/*---------------------------------------------------------*/

		/* Operators ----------------------------------------------*/
		Vec2 operator*(const float& rhs) const noexcept
		{
			return Vec2(this->x * rhs, this->y * rhs);
		}
		Vec2 operator/(const float& rhs) const noexcept
		{
			return Vec2(this->x / rhs, this->y / rhs);
		}
		Vec2 operator+(const Vec2& rhs) const noexcept
		{
			return Vec2(this->x + rhs.x, this->y + rhs.y);
		}
		Vec2 operator-(const Vec2& rhs) const noexcept
		{
			return Vec2(this->x - rhs.x, this->y - rhs.y);
		}
		Vec2 operator-() const noexcept
		{
			return Vec2(-this->x, -this->y);
		}
		Vec2& operator*=(float rhs) noexcept
		{
			this->x *= rhs;
			this->y *= rhs;
			return *this;
		}
		Vec2& operator/=(const float& rhs) noexcept
		{
			this->x /= rhs;
			this->y /= rhs;
			return *this;
		}
		Vec2& operator+=(const Vec2& rhs) noexcept
		{
			this->x += rhs.x;
			this->y += rhs.y;
			return *this;
		}
		Vec2& operator-=(const Vec2& rhs) noexcept
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			return *this;
		}
		bool operator==(const Vec2& rhs) const noexcept
		{
			return (this->x == rhs.x && this->y == rhs.y);
		}
		/*---------------------------------------------------------*/
	};

	/*-------------------------------------------------------------------------*/
	/* 3-Dimensional Vectors												   */
	/*-------------------------------------------------------------------------*/
	struct Vec3
	{
		float x, y, z;

		/* Constructors -------------------------------------------*/
		Vec3()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
		}
		Vec3(float v)
		{
			this->x = v;
			this->y = v;
			this->z = v;
		}
		Vec3(Vec2 vec2, float z)
		{
			this->x = vec2.x;
			this->y = vec2.y;
			this->z = z;
		}
		Vec3(float x, Vec2 vec2)
		{
			this->x = x;
			this->y = vec2.x;
			this->z = vec2.y;
		}
		Vec3(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		/*---------------------------------------------------------*/

		/* Operators ----------------------------------------------*/
		Vec3 operator*(const float& rhs) const noexcept
		{
			return Vec3(this->x * rhs, this->y * rhs, this->z * rhs);
		}
		Vec3 operator/(const float& rhs) const noexcept
		{
			return Vec3(this->x / rhs, this->y / rhs, this->z / rhs);
		}
		Vec3 operator+(const Vec3& rhs) const noexcept
		{
			return Vec3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
		}
		Vec3 operator-(const Vec3& rhs) const noexcept
		{
			return Vec3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
		}
		Vec3 operator-() const noexcept
		{
			return Vec3(-this->x, -this->y, -this->z);
		}
		Vec3& operator*=(const float& rhs) noexcept
		{
			this->x *= rhs;
			this->y *= rhs;
			this->z *= rhs;
			return *this;
		}
		Vec3& operator/=(const float& rhs) noexcept
		{
			this->x /= rhs;
			this->y /= rhs;
			this->z /= rhs;
			return *this;
		}
		Vec3& operator+=(const Vec3& rhs) noexcept
		{
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;
			return *this;
		}
		Vec3& operator-=(const Vec3& rhs) noexcept
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			this->z -= rhs.z;
			return *this;
		}
		bool operator==(const Vec3& rhs) const noexcept
		{
			return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z);
		}
		/*---------------------------------------------------------*/
	};

	/*-------------------------------------------------------------------------*/
	/* 4-Dimensional Vectors												   */
	/*-------------------------------------------------------------------------*/
	struct Vec4
	{
		float x, y, z, w;

		/* Constructors -------------------------------------------*/
		Vec4()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 0;
		}
		Vec4(float v)
		{
			this->x = v;
			this->y = v;
			this->z = v;
			this->w = v;
		}
		Vec4(Vec2 vec2, float z, float w)
		{
			this->x = vec2.x;
			this->y = vec2.y;
			this->z = z;
			this->w = w;
		}
		Vec4(float x, float y, Vec2 vec2)
		{
			this->x = x;
			this->y = y;
			this->z = vec2.x;
			this->w = vec2.y;
		}
		Vec4(Vec2 a, Vec2 b)
		{
			this->x = a.x;
			this->y = a.y;
			this->z = a.x;
			this->w = a.y;
		}
		Vec4(Vec3 vec3, float w)
		{
			this->x = vec3.x;
			this->y = vec3.y;
			this->z = vec3.z;
			this->w = w;
		}
		Vec4(float x, Vec3 vec3)
		{
			this->x = x;
			this->y = vec3.x;
			this->z = vec3.y;
			this->w = vec3.z;
		}
		Vec4(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		/*---------------------------------------------------------*/

		/* Operators ----------------------------------------------*/
		Vec4 operator*(const float& rhs) const noexcept
		{
			return Vec4(this->x * rhs, this->y * rhs, this->z * rhs, this->w * rhs);
		}
		Vec4 operator/(const float& rhs) const noexcept
		{
			return Vec4(this->x / rhs, this->y / rhs, this->z / rhs, this->w / rhs);
		}
		Vec4 operator+(const Vec4& rhs) const noexcept
		{
			return Vec4(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z, this->w + rhs.w);
		}
		Vec4 operator-(const Vec4& rhs) const noexcept
		{
			return Vec4(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z, this->w - rhs.w);
		}
		Vec4 operator-() const noexcept
		{
			return Vec4(-this->x, -this->y, -this->z, -this->w);
		}
		Vec4& operator*=(const float& rhs) noexcept
		{
			this->x *= rhs;
			this->y *= rhs;
			this->z *= rhs;
			this->w *= rhs;
			return *this;
		}
		Vec4& operator/=(const float& rhs) noexcept
		{
			this->x /= rhs;
			this->y /= rhs;
			this->z /= rhs;
			this->w /= rhs;
			return *this;
		}
		Vec4& operator+=(const Vec4& rhs) noexcept
		{
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;
			this->w += rhs.w;
			return *this;
		}
		Vec4& operator-=(const Vec4& rhs) noexcept
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			this->z -= rhs.z;
			this->w -= rhs.w;
			return *this;
		}
		bool operator==(const Vec4& rhs) const noexcept
		{
			return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z && this->w == rhs.w);
		}
		/*---------------------------------------------------------*/
	};

	/*-------------------------------------------------------------------------*/
	/* Quaternions															   */
	/*-------------------------------------------------------------------------*/
	struct Quaternion
	{
		float x, y, z, w;

		/* Constructors -------------------------------------------*/
		Quaternion()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 1;
		}
		Quaternion(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		Quaternion(Vec4 vec4)
		{
			this->x = vec4.x;
			this->y = vec4.y;
			this->z = vec4.z;
			this->w = vec4.w;
		}
		Quaternion(Vec3 axis, float angle)
		{
			float h = angle * 0.5f;
			Vec3 v = axis * sin(h);
			
			this->w = cos(h);
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
		}
		/*---------------------------------------------------------*/

		/* Operators ----------------------------------------------*/
		operator Vec4() { return Vec4(x, y, z, w); }
		bool operator==(const Quaternion& rhs) const noexcept
		{
			return ((this->w == rhs.w) && (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z));
		}
		Quaternion operator-() const noexcept
		{
			return Quaternion{ -this->x, -this->y, -this->z, -this->w };
		}
		Quaternion operator*(const Quaternion& rhs) const noexcept
		{
			return {	(this->w * rhs.x + this->x * rhs.w + this->y * rhs.z - this->z * rhs.y),
						(this->w * rhs.y - this->x * rhs.z + this->y * rhs.w + this->z * rhs.x),
						(this->w * rhs.z + this->x * rhs.y - this->y * rhs.x + this->z * rhs.w),
						(this->w * rhs.w - this->x * rhs.x - this->y * rhs.y - this->z * rhs.z)
			};
		}
		/*---------------------------------------------------------*/
	};

	/*-------------------------------------------------------------------------*/
	/* Vertices																   */
	/*-------------------------------------------------------------------------*/
	struct Vertex
	{
		Vec3	position;
		Vec4	color;

		/* Binding Description ------------------------------------*/
		static VkVertexInputBindingDescription BindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		/* Attribute Description ----------------------------------*/
		static std::vector<VkVertexInputAttributeDescription> AttributeDescriptions()
		{
			VkVertexInputAttributeDescription attDesc0{};
			attDesc0.binding = 0;
			attDesc0.location = 0;
			attDesc0.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attDesc0.offset = offsetof(Vertex, position);

			VkVertexInputAttributeDescription attDesc1{};
			attDesc1.binding = 0;
			attDesc1.location = 1;
			attDesc1.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attDesc1.offset = offsetof(Vertex, color);

			return { attDesc0, attDesc1 };
		}
	};

	/*-------------------------------------------------------------------------*/
	/* Triangles															   */
	/*-------------------------------------------------------------------------*/
	struct Triangle
	{
		Vertex a, b, c;
	};
}

#endif
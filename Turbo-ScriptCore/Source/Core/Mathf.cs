﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	public struct Vector2
	{
		public float x;
		public float y;

		public Vector2(float _x, float _y)
		{
			x = _x;
			y = _y;
		}
		public Vector2(float xy) : this(xy, xy) { }

		public override string ToString()
		{
			return $"(x: {x}, y: {y})";
		}
		public static Vector2 Zero => new Vector2(0.0f, 0.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public static Vector2 operator +(Vector2 u, Vector2 v)
		{
			return new Vector2(u.x + v.x, u.y + v.y);
		}

		public static Vector2 operator -(Vector2 u, Vector2 v)
		{
			return new Vector2(u.x - v.x, u.y - v.y);
		}

		public static Vector2 operator *(Vector2 u, float v)
		{
			return new Vector2(u.x * v, u.y * v);
		}
	}

	public struct Vector3
	{
		public float x;
		public float y;
		public float z;

		public Vector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		public Vector3(Vector3 other) : this(other.x, other.y, other.z) { }

		public Vector3(float xyz) : this(xyz, xyz, xyz) { }

		public static Vector3 Zero => new Vector3(0.0f, 0.0f, 0.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, 1.0f);

		public static Vector3 operator +(Vector3 u, Vector3 v)
		{
			return new Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
		}

		public static Vector3 operator -(Vector3 u, Vector3 v)
		{
			return new Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
		}

		public static Vector3 operator *(Vector3 u, float v)
		{
			return new Vector3(u.x * v, u.y * v, u.z * v);
		}

		public override string ToString()
		{
			return $"(x: {x}, y: {y}, z: {z})";
		}
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct Vector4
	{
		[FieldOffset(0)] public float x;
		[FieldOffset(4)] public float y;
		[FieldOffset(8)] public float z;
		[FieldOffset(12)] public float w;

		[FieldOffset(0)] public float r;
		[FieldOffset(4)] public float g;
		[FieldOffset(8)] public float b;
		[FieldOffset(12)] public float a;

		public Vector4(float _x, float _y, float _z, float _w)
		{
			r = x = _x;
			g = y = _y;
			b = z = _z;
			a = w = _w;
		}
		public Vector4(float xyzw) : this(xyzw, xyzw, xyzw, xyzw) { }

		public override string ToString()
		{
			return $"(x: {x}, y: {y}, z: {z}, w: {w})";
		}
	}

	public static class Mathf
	{
		public const float PI = 3.1415926535897931f;

		public static float Radians(float angle)
		{
			return (Mathf.PI / 180.0f) * angle;
		}

		// Trigonometric functions
		public static float Sin(float radians)
		{
			return (float)Math.Sin(radians);
		}
		public static float Cos(float radians)
		{
			return (float)Math.Cos(radians);
		}
		public static float Tan(float radians)
		{
			return (float)Math.Tan(radians);
		}
		public static float Sinh(float radians)
		{
			return (float)Math.Sinh(radians);
		}
		public static float Cosh(float radians)
		{
			return (float)Math.Cosh(radians);
		}

		public static float Tanh(float radians)
		{
			return (float)Math.Tanh(radians);
		}

		// Inverse trigonometric functions
		public static float Asin(float value)
		{
			return (float)Math.Asin(value);
		}
		public static float Acos(float value)
		{
			return (float)Math.Acos(value);
		}
		public static float Atan(float value)
		{
			return (float)Math.Atan(value);
		}
	}
}

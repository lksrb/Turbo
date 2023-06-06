using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Turbo
{
	public static class Application
	{
		public static uint GetWidth() => InternalCalls.Application_GetWidth();
		public static uint GetHeight() => InternalCalls.Application_GetHeight();
		public static void Close() => InternalCalls.Application_Close();
	}
}

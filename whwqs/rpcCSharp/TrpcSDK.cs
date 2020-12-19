using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace rpcCSharp
{
	[StructLayout(LayoutKind.Sequential)]
	public struct TrpcInOut
	{
		[MarshalAs(UnmanagedType.U4)]
		public int length;
		//[MarshalAs(UnmanagedType.LPStr)]
		public IntPtr buffer;
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate TrpcInOut RequestCallback(TrpcInOut input);

	[StructLayout(LayoutKind.Sequential)]
	public struct TrpcServerInit
	{
		[MarshalAs(UnmanagedType.U1)]
		public bool commit;
		[MarshalAs(UnmanagedType.FunctionPtr)]
		public RequestCallback requestcbk;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 20)]
		public string dataFile;
		[MarshalAs(UnmanagedType.Struct)]
		public _SRpcInit rpcInit;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct TrpcEpSet
	{
		[MarshalAs(UnmanagedType.U1)]
		public sbyte inUse;
		[MarshalAs(UnmanagedType.U1)]
		public sbyte numOfEps;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.U2)]
		public ushort[] port;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.BStr)]
		public IntPtr[] fqdn;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct _SRpcInit
	{
		[MarshalAs(UnmanagedType.U2)]
		public ushort localPort;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 8)]
		public string label;
		[MarshalAs(UnmanagedType.U4)]
		public int numOfThreads;
		[MarshalAs(UnmanagedType.U4)]
		public int sessions;
		[MarshalAs(UnmanagedType.U1)]
		public byte connType;
		[MarshalAs(UnmanagedType.U4)]
		public int idleTime;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 24)]
		public string user;
		[MarshalAs(UnmanagedType.U1)]
		public char spi;
		[MarshalAs(UnmanagedType.U1)]
		public char encrypt;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string secret;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string ckey;
	}

	public class TrpcSDK_WINDOWS
	{

		[DllImport("trpcdylib.dll", EntryPoint = "StartServerListen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr StartServerListen(TrpcServerInit trpcServerInit);

		[DllImport("trpcdylib.dll", EntryPoint = "ClientSendAndReceive", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr ClientSendAndReceive(IntPtr pRpc, TrpcEpSet epSet, TrpcInOut input);

		[DllImport("trpcdylib.dll", EntryPoint = "_RpcOpen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr _RpcOpen(_SRpcInit rpcInit);

		[DllImport("trpcdylib.dll", EntryPoint = "_RpcClose", CallingConvention = CallingConvention.Cdecl)]
		static extern public void _RpcClose(IntPtr param);

		[DllImport("trpcdylib.dll", EntryPoint = "InitLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public int InitLog(string logName, int numOfLogLines, int maxFiles);

		[DllImport("trpcdylib.dll", EntryPoint = "SetDebug", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetDebug(int rpcDebugFlag);

		[DllImport("trpcdylib.dll", EntryPoint = "CloseLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void CloseLog();

		[DllImport("trpcdylib.dll", EntryPoint = "ResetLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void ResetLog();

		[DllImport("trpcdylib.dll", EntryPoint = "SetCompressMsgSize", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetCompressMsgSize(int CompressMsgSize);

		[DllImport("trpcdylib.dll", EntryPoint = "FreeTrpcInOut", CallingConvention = CallingConvention.Cdecl)]
		static extern public void FreeTrpcInOut(IntPtr param);
	}

	public class TrpcSDK_LINUX
	{

		[DllImport("libtrpcdylib.so", EntryPoint = "StartServerListen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr StartServerListen(TrpcServerInit trpcServerInit);

		[DllImport("libtrpcdylib.so", EntryPoint = "ClientSendAndReceive", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr ClientSendAndReceive(IntPtr pRpc, TrpcEpSet epSet, TrpcInOut input);

		[DllImport("libtrpcdylib.so", EntryPoint = "_RpcOpen", CallingConvention = CallingConvention.Cdecl)]
		static extern public IntPtr _RpcOpen(_SRpcInit rpcInit);

		[DllImport("libtrpcdylib.so", EntryPoint = "_RpcClose", CallingConvention = CallingConvention.Cdecl)]
		static extern public void _RpcClose(IntPtr param);

		[DllImport("libtrpcdylib.so", EntryPoint = "InitLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public int InitLog(string logName, int numOfLogLines, int maxFiles);

		[DllImport("libtrpcdylib.so", EntryPoint = "SetDebug", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetDebug(int rpcDebugFlag);

		[DllImport("libtrpcdylib.so", EntryPoint = "CloseLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void CloseLog();

		[DllImport("libtrpcdylib.so", EntryPoint = "ResetLog", CallingConvention = CallingConvention.Cdecl)]
		static extern public void ResetLog();

		[DllImport("libtrpcdylib.so", EntryPoint = "SetCompressMsgSize", CallingConvention = CallingConvention.Cdecl)]
		static extern public void SetCompressMsgSize(int CompressMsgSize);

		[DllImport("libtrpcdylib.so", EntryPoint = "FreeTrpcInOut", CallingConvention = CallingConvention.Cdecl)]
		static extern public void FreeTrpcInOut(IntPtr param);
	}

	public class TrpcSDK
	{
		static public IntPtr StartServerListen(TrpcServerInit trpcServerInit)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				return TrpcSDK_LINUX.StartServerListen(trpcServerInit);
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				return TrpcSDK_WINDOWS.StartServerListen(trpcServerInit);
			}

			return IntPtr.Zero;
		}

		static public IntPtr ClientSendAndReceive(IntPtr pRpc, TrpcEpSet epSet, TrpcInOut input)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				return TrpcSDK_LINUX.ClientSendAndReceive(pRpc, epSet, input);
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				return TrpcSDK_WINDOWS.ClientSendAndReceive(pRpc, epSet, input);
			}

			return IntPtr.Zero;
		}

		static public IntPtr _RpcOpen(_SRpcInit rpcInit)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				return TrpcSDK_LINUX._RpcOpen(rpcInit);
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				return TrpcSDK_WINDOWS._RpcOpen(rpcInit);
			}

			return IntPtr.Zero;
		}

		static public void _RpcClose(IntPtr param)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX._RpcClose(param);
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS._RpcClose(param);
				return;
			}
		}

		static public int InitLog(string logName, int numOfLogLines, int maxFiles)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				return TrpcSDK_LINUX.InitLog(logName, numOfLogLines, maxFiles);
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				return TrpcSDK_WINDOWS.InitLog(logName, numOfLogLines, maxFiles);
			}

			return -99999;
		}

		static public void SetDebug(int rpcDebugFlag)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX.SetDebug(rpcDebugFlag);
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS.SetDebug(rpcDebugFlag);
				return;
			}
		}

		static public void CloseLog()
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX.CloseLog();
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS.CloseLog();
				return;
			}
		}

		static public void ResetLog()
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX.ResetLog();
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS.ResetLog();
				return;
			}
		}

		static public void SetCompressMsgSize(int CompressMsgSize)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX.SetCompressMsgSize(CompressMsgSize);
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS.SetCompressMsgSize(CompressMsgSize);
				return;
			}
		}

		public static void FreeTrpcInOut(IntPtr param)
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
			{
				TrpcSDK_LINUX.FreeTrpcInOut(param);
				return;
			}

			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				TrpcSDK_WINDOWS.FreeTrpcInOut(param);
				return;
			}
		}
	}
}

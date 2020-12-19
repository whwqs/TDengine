using rpcCSharp;
using System;

namespace TrpcServer
{
	class Program
	{
		static IntPtr 服务端rpc;
		static int 服务端接收消息条数 = 0;
		static RequestCallback serverCallback;
		static TrpcInOut requestCallback(TrpcInOut input)
		{
			TrpcInOut output = new TrpcInOut();
			if (input.length > 0)
			{
				string 服务端接收的最后消息 = TrpcTools.Utf8BufferPtrToString(input.buffer, input.length);
				Console.WriteLine(服务端接收的最后消息);
				服务端接收消息条数++;
				string ret = $"服务端收到第{服务端接收消息条数}条消息：" + 服务端接收的最后消息 + "，并对消息进行处理。";				
				byte[] buf = TrpcTools.StringToUtf8Buffer(ret);
				output.length = buf.Length;
				output.buffer = TrpcTools.BytesToIntptr(buf);
			}
			return output;
		}
		static void Main(string[] args)
		{
			serverCallback = requestCallback;
			TrpcSDK.InitLog("server.log", 100000, 10);
			TrpcSDK.SetDebug(131);
			//TrpcSDK.SetCompressMsgSize(0);
			_SRpcInit rpcInit = new _SRpcInit();
			rpcInit.connType = 0;
			rpcInit.idleTime = 1;
			rpcInit.label = "server";
			rpcInit.localPort = 7000;
			rpcInit.numOfThreads = 5;
			rpcInit.sessions = 1000;
			TrpcServerInit trpcServerInit = new TrpcServerInit();
			trpcServerInit.commit = false;
			trpcServerInit.dataFile = "server.data";
			trpcServerInit.requestcbk = serverCallback;
			trpcServerInit.rpcInit = rpcInit;
			服务端rpc = TrpcSDK.StartServerListen(trpcServerInit);
			if (服务端rpc != IntPtr.Zero)
			{
				Console.WriteLine("服务端开启监听成功，按q键退出！");
			}
			else
			{
				Console.WriteLine("服务端开启监听失败");
				return;
			}
			while (true)
			{
				string key = Console.ReadLine();
				if (key.ToUpper().StartsWith('Q'))
				{
					TrpcSDK._RpcClose(服务端rpc);
					break;
				}
			}
		}
	}
}

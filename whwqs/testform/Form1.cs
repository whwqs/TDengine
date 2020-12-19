using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using rpcCSharp;

namespace testform
{	
	public partial class Form1 : Form
	{
		SynchronizationContext cxt;
		RequestCallback serverCallback;
		IntPtr 客户端rpc;
		IntPtr 服务端rpc;
		public Form1()
		{
			InitializeComponent();
			this.Load += Form1_Load;
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			cxt = SynchronizationContext.Current;
			serverCallback = requestCallback;
		}

		private void 开启服务监听_Click(object sender, EventArgs e)
		{
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
				lbl服务监听状态.Text = "服务端开启监听成功";
			}
			else
			{
				lbl服务监听状态.Text = "服务端开启监听失败" ;
			}
		}

		int 服务端接收消息条数 = 0;
		string 服务端接收的最后消息 = "";

		static object loc1 = new object();
		void 服务端消息打印()
		{
			lock (loc1)
			{
				cxt.Post(o =>
				{
					txtRec1.Text = $"{Environment.NewLine}最新客户端消息是：{Environment.NewLine + 服务端接收的最后消息 + Environment.NewLine}收到消息的总条数：{Environment.NewLine + 服务端接收消息条数}";
				}, null);
			}
		}

		private TrpcInOut requestCallback(TrpcInOut input)
		{
			TrpcInOut output = new TrpcInOut();
			if (input.length > 0)
			{
				服务端接收的最后消息 = TrpcTools.Utf8BufferPtrToString(input.buffer, input.length);
				服务端接收消息条数++;
				string ret = $"服务端收到第{服务端接收消息条数}条消息：" + 服务端接收的最后消息 + "，并对消息进行处理。";
				byte[] buf = TrpcTools.StringToUtf8Buffer(ret);
				output.length = buf.Length;
				output.buffer = TrpcTools.BytesToIntptr(buf);
				Task.Factory.StartNew(服务端消息打印);
			}
			return output;
		}


		int 客户端接收消息条数 = 0;
		string 客户端接收的最后消息 = "";
		double 发送接收总时间 = 0;
		double 最后一次发送接收总时间 = 0;		

		private void 客户端发送并接收_Click(object sender, EventArgs e)
		{	
			客户端接收消息条数 = 0;
			客户端接收的最后消息 = "";
			发送接收总时间 = 0;
			最后一次发送接收总时间 = 0;			
			txtRec2.Text = "";
			Application.DoEvents();

			TrpcSDK.InitLog("c1ient.log", 100000, 10);
			TrpcSDK.SetDebug(131);
			//TrpcSDK.SetCompressMsgSize(0);
			TrpcEpSet serverEpSet = new TrpcEpSet();
			serverEpSet.port = new ushort[] { 7000,0,0,0,0 };
			serverEpSet.fqdn = new IntPtr[5] {  Marshal.StringToHGlobalAnsi(txtServerIp.Text),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi(""),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi("") };
			serverEpSet.numOfEps = 5;
			serverEpSet.inUse = 0;
			int n = int.Parse(txtTimes.Text);			

			for (int i = 1; i <= n; i++)
			{				
				Stopwatch sw = new Stopwatch();
				sw.Start();
				TrpcInOut input = new TrpcInOut();
				byte[] buf = TrpcTools.StringToUtf8Buffer(txtSend.Text);
				input.length = buf.Length;
				input.buffer = TrpcTools.BytesToIntptr(buf);
				IntPtr ptr = TrpcSDK.ClientSendAndReceive(客户端rpc, serverEpSet,input);//
				TrpcInOut output = (TrpcInOut)Marshal.PtrToStructure(ptr, typeof(TrpcInOut));
				string respMsg = TrpcTools.Utf8BufferPtrToString(output.buffer, output.length);
				TrpcSDK.FreeTrpcInOut(ptr);
				sw.Stop();
				最后一次发送接收总时间 = sw.Elapsed.TotalSeconds;
				发送接收总时间 += 最后一次发送接收总时间;
				客户端接收消息条数++;
				客户端接收的最后消息 = respMsg;				
			}

			txtRec2.Text = $"{Environment.NewLine}最新服务端消息是：{Environment.NewLine + 客户端接收的最后消息 + Environment.NewLine}收到消息的总条数：{Environment.NewLine + 客户端接收消息条数 + Environment.NewLine}最后消息发送接收花费秒数是：{Environment.NewLine + 最后一次发送接收总时间 + Environment.NewLine}总发送接收秒数是：{Environment.NewLine + 发送接收总时间}";
		}

		private void 创建客户端rpc连接_Click(object sender, EventArgs e)
		{
			_SRpcInit rpcInit = new _SRpcInit();
			rpcInit.ckey = "*";
			rpcInit.connType = 1;
			rpcInit.encrypt = '*';
			rpcInit.idleTime = 1;
			rpcInit.label = "client";
			rpcInit.localPort = 0;
			rpcInit.numOfThreads = 1;
			rpcInit.secret = "####";
			rpcInit.sessions = 1000;
			//rpcInit.spi = '%';
			rpcInit.user = "wqs";
			客户端rpc = TrpcSDK._RpcOpen(rpcInit);
			if (客户端rpc != IntPtr.Zero)
			{
				lbl客户端rpc创建状态.Text = "客户端rpc创建成功";
			}
			else
			{
				lbl客户端rpc创建状态.Text = "客户端rpc创建失败";
			}
		}

		private void 关闭服务监听_Click(object sender, EventArgs e)
		{
			if (服务端rpc != IntPtr.Zero)
			{
				TrpcSDK._RpcClose(服务端rpc);
				服务端rpc = IntPtr.Zero;
				lbl服务监听状态.Text = "服务端关闭监听成功";
			}
		}

		private void 关闭客户端rpc_Click(object sender, EventArgs e)
		{
			if (客户端rpc != IntPtr.Zero)
			{
				TrpcSDK._RpcClose(客户端rpc);
				客户端rpc = IntPtr.Zero;
				lbl客户端rpc创建状态.Text = "客户端rpc关闭成功";
			}
		}
	}
}

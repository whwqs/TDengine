﻿using System;
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
		ResponseCallback clientCallback;
		IntPtr 客户端rpc;
		IntPtr 服务端rpc;
		private static object 客户端rpc锁 = new object();
		public Form1()
		{
			InitializeComponent();
			this.Load += Form1_Load;
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			cxt = SynchronizationContext.Current;
			serverCallback = requestCallback;
			clientCallback = responseCallback;
		}

		private void 开启服务监听_Click(object sender, EventArgs e)
		{
			TrpcSDK.InitLog("server.log", 100000, 10);
			TrpcSDK.SetDebug(131);
			TrpcSDK.SetCompressMsgSize(10);
			_SRpcInit rpcInit = new _SRpcInit();
			rpcInit.connType = 0;
			rpcInit.idleTime = 5000;
			rpcInit.label = "server";
			rpcInit.localPort = 7000;
			rpcInit.numOfThreads = 1;
			rpcInit.sessions = 10000;
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
				lbl服务监听状态.Text = "服务端开启监听失败";
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
					txtRec1.Text = $"收到消息的总条数：{Environment.NewLine + 服务端接收消息条数}{Environment.NewLine}最新客户端消息是：{Environment.NewLine + 服务端接收的最后消息 + Environment.NewLine}";
				}, null);
			}
		}

		static object loc2 = new object();
		void 客户端消息打印()
		{
			lock (loc2)
			{
				cxt.Post(o =>
				{
					txtRec2.Text = $"{Environment.NewLine}最新服务端消息是：{Environment.NewLine + 客户端接收的最后消息 + Environment.NewLine}收到消息的总条数：{Environment.NewLine + 客户端接收消息条数 + Environment.NewLine}最后消息发送接收花费秒数是：{Environment.NewLine + 最后一次发送接收总时间 + Environment.NewLine}总发送接收秒数是：{Environment.NewLine + 发送接收总时间}";
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

		private void responseCallback(TrpcInOut input)
		{
			客户端接收的最后消息 = TrpcTools.Utf8BufferPtrToString(input.buffer, input.length);
		}


		int 客户端接收消息条数 = 0;
		string 客户端接收的最后消息 = "";
		double 发送接收总时间 = 0;
		double 最后一次发送接收总时间 = 0;

		private void 客户端发送并接收_Click(object sender, EventArgs e)
		{
			if (客户端rpc == IntPtr.Zero)
			{
				MessageBox.Show("请先开启客户端");
				return;
			}

			客户端接收消息条数 = 0;
			客户端接收的最后消息 = "";
			发送接收总时间 = 0;
			最后一次发送接收总时间 = 0;
			txtRec2.Text = "";
			Application.DoEvents();

			TrpcSDK.InitLog("c1ient.log", 100000, 10);
			TrpcSDK.SetDebug(131);
			TrpcSDK.SetCompressMsgSize(10);
			TrpcEpSet serverEpSet = new TrpcEpSet();
			serverEpSet.port = new ushort[] { 7000, 0, 0, 0, 0 };
			serverEpSet.fqdn = new IntPtr[5] {  Marshal.StringToHGlobalAnsi(txtServerIp.Text),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi(""),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi("") };
			serverEpSet.numOfEps = 1;
			serverEpSet.inUse = 0;
			int n = int.Parse(txtTimes.Text);

			Task.Factory.StartNew(() =>
			{
				for (int i = 1; i <= n; i++)
				{
					if (客户端rpc == IntPtr.Zero) break;
					Stopwatch sw = new Stopwatch();
					sw.Start();
					TrpcInOut input = new TrpcInOut();
					byte[] buf = TrpcTools.StringToUtf8Buffer("" + i + " " + txtSend.Text);
					input.length = buf.Length;
					input.buffer = TrpcTools.BytesToIntptr(buf);
					TrpcSDK.ClientSendAndReceive(客户端rpc, serverEpSet, input, clientCallback);//
					sw.Stop();
					最后一次发送接收总时间 = sw.Elapsed.TotalSeconds;
					发送接收总时间 += 最后一次发送接收总时间;
					客户端接收消息条数++;
					客户端消息打印();
				}
			});

		}

		private void 创建客户端rpc连接_Click(object sender, EventArgs e)
		{
			_SRpcInit rpcInit = new _SRpcInit();
			rpcInit.ckey = "*";
			rpcInit.connType = 1;
			rpcInit.encrypt = '*';
			rpcInit.idleTime = 2000;
			rpcInit.label = "client";
			rpcInit.localPort = 0;
			rpcInit.numOfThreads = 1;
			rpcInit.secret = "####";
			rpcInit.sessions = 1;
			rpcInit.spi = 0;
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

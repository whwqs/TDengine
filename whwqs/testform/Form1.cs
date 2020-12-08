using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

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
			//TrpcSDK.SetCompressMsgSize(-1);
			_SRpcInit rpcInit = new _SRpcInit();
			rpcInit.connType = 0;
			rpcInit.idleTime = 1;
			rpcInit.label = "server";
			rpcInit.localPort = 7000;
			rpcInit.numOfThreads = 5;
			rpcInit.sessions = 100;
			TrpcServerInit trpcServerInit = new TrpcServerInit();
			trpcServerInit.commit = false;
			trpcServerInit.dataFile = "server.data";
			trpcServerInit.requestcbk = serverCallback;
			trpcServerInit.rpcInit = rpcInit;
			服务端rpc = TrpcSDK.StartServerListen(trpcServerInit);
			if (服务端rpc != IntPtr.Zero)
			{
				txtRec1.Text = "服务端开启监听成功" + Environment.NewLine;
			}
			else
			{
				txtRec1.Text = "服务端开启监听失败" + Environment.NewLine;
			}
		}

		private string requestCallback(string content)
		{
			string ret = "服务端收到：" + content + "_me";
			//cxt.Post(o => {
			//	txtRec1.Text += content + Environment.NewLine;
			//}, null);
			return ret;
		}		

		private void 客户端发送并接收_Click(object sender, EventArgs e)
		{
			TrpcSDK.InitLog("c1ient.log", 100000, 10);
			TrpcSDK.SetDebug(131);
			TrpcEpSet serverEpSet = new TrpcEpSet();
			serverEpSet.port = new ushort[] { 7000,10,10,10,110 };
			serverEpSet.fqdn = new IntPtr[5] {  Marshal.StringToHGlobalAnsi("127.0.0.1"),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi(""),  Marshal.StringToHGlobalAnsi(""),
				 Marshal.StringToHGlobalAnsi("") };
			serverEpSet.numOfEps = 5;
			serverEpSet.inUse = 0;
			string resp = "";
			int n = int.Parse(txtTimes.Text);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < n; i++)
			{				
				resp = TrpcSDK.ClientSendAndReceive(客户端rpc, serverEpSet, txtSend.Text + i);
				sb.Append(resp + Environment.NewLine);				
			}
			txtRec2.Text += sb.ToString();
			MessageBox.Show(resp);
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
			rpcInit.sessions = 100;
			//rpcInit.spi = '%';
			rpcInit.user = "wqs";
			客户端rpc = TrpcSDK._RpcOpen(rpcInit);
		}

		private void 关闭服务监听_Click(object sender, EventArgs e)
		{
			if (服务端rpc != IntPtr.Zero)
			{
				TrpcSDK._RpcClose(服务端rpc);
			}
		}

		private void 关闭客户端rpc_Click(object sender, EventArgs e)
		{
			if (客户端rpc != IntPtr.Zero)
			{
				TrpcSDK._RpcClose(客户端rpc);
			}
		}
	}
}

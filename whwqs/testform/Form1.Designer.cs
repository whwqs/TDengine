namespace testform
{
	partial class Form1
	{
		/// <summary>
		/// 必需的设计器变量。
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// 清理所有正在使用的资源。
		/// </summary>
		/// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows 窗体设计器生成的代码

		/// <summary>
		/// 设计器支持所需的方法 - 不要修改
		/// 使用代码编辑器修改此方法的内容。
		/// </summary>
		private void InitializeComponent()
		{
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.txtRec1 = new System.Windows.Forms.TextBox();
			this.txtSend = new System.Windows.Forms.TextBox();
			this.txtRec2 = new System.Windows.Forms.TextBox();
			this.txtTimes = new System.Windows.Forms.TextBox();
			this.btnRpcOpen = new System.Windows.Forms.Button();
			this.button4 = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.button3 = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(58, 37);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(100, 23);
			this.button1.TabIndex = 0;
			this.button1.Text = "开启服务监听";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.开启服务监听_Click);
			// 
			// button2
			// 
			this.button2.Location = new System.Drawing.Point(116, 332);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(75, 79);
			this.button2.TabIndex = 1;
			this.button2.Text = "客户端发送消息并接收结果";
			this.button2.UseVisualStyleBackColor = true;
			this.button2.Click += new System.EventHandler(this.客户端发送并接收_Click);
			// 
			// txtRec1
			// 
			this.txtRec1.Location = new System.Drawing.Point(206, 37);
			this.txtRec1.Multiline = true;
			this.txtRec1.Name = "txtRec1";
			this.txtRec1.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtRec1.Size = new System.Drawing.Size(538, 217);
			this.txtRec1.TabIndex = 2;
			// 
			// txtSend
			// 
			this.txtSend.Location = new System.Drawing.Point(206, 315);
			this.txtSend.Multiline = true;
			this.txtSend.Name = "txtSend";
			this.txtSend.Size = new System.Drawing.Size(538, 123);
			this.txtSend.TabIndex = 3;
			// 
			// txtRec2
			// 
			this.txtRec2.Location = new System.Drawing.Point(206, 497);
			this.txtRec2.Multiline = true;
			this.txtRec2.Name = "txtRec2";
			this.txtRec2.ScrollBars = System.Windows.Forms.ScrollBars.Both;
			this.txtRec2.Size = new System.Drawing.Size(538, 217);
			this.txtRec2.TabIndex = 4;
			// 
			// txtTimes
			// 
			this.txtTimes.Location = new System.Drawing.Point(12, 362);
			this.txtTimes.Name = "txtTimes";
			this.txtTimes.Size = new System.Drawing.Size(66, 21);
			this.txtTimes.TabIndex = 5;
			this.txtTimes.Text = "100";
			// 
			// btnRpcOpen
			// 
			this.btnRpcOpen.Location = new System.Drawing.Point(21, 266);
			this.btnRpcOpen.Name = "btnRpcOpen";
			this.btnRpcOpen.Size = new System.Drawing.Size(137, 23);
			this.btnRpcOpen.TabIndex = 7;
			this.btnRpcOpen.Text = "创建客户端rpc";
			this.btnRpcOpen.UseVisualStyleBackColor = true;
			this.btnRpcOpen.Click += new System.EventHandler(this.创建客户端rpc连接_Click);
			// 
			// button4
			// 
			this.button4.Location = new System.Drawing.Point(58, 82);
			this.button4.Name = "button4";
			this.button4.Size = new System.Drawing.Size(100, 23);
			this.button4.TabIndex = 8;
			this.button4.Text = "关闭服务监听";
			this.button4.UseVisualStyleBackColor = true;
			this.button4.Click += new System.EventHandler(this.关闭服务监听_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(93, 365);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(17, 12);
			this.label1.TabIndex = 9;
			this.label1.Text = "次";
			// 
			// button3
			// 
			this.button3.Location = new System.Drawing.Point(175, 266);
			this.button3.Name = "button3";
			this.button3.Size = new System.Drawing.Size(116, 23);
			this.button3.TabIndex = 10;
			this.button3.Text = "关闭客户端rpc";
			this.button3.UseVisualStyleBackColor = true;
			this.button3.Click += new System.EventHandler(this.关闭客户端rpc_Click);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(800, 773);
			this.Controls.Add(this.button3);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.button4);
			this.Controls.Add(this.btnRpcOpen);
			this.Controls.Add(this.txtTimes);
			this.Controls.Add(this.txtRec2);
			this.Controls.Add(this.txtSend);
			this.Controls.Add(this.txtRec1);
			this.Controls.Add(this.button2);
			this.Controls.Add(this.button1);
			this.Name = "Form1";
			this.Text = "Form1";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.TextBox txtRec1;
		private System.Windows.Forms.TextBox txtSend;
		private System.Windows.Forms.TextBox txtRec2;
		private System.Windows.Forms.TextBox txtTimes;
		private System.Windows.Forms.Button btnRpcOpen;
		private System.Windows.Forms.Button button4;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button button3;
	}
}


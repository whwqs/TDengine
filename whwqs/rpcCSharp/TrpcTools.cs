using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace rpcCSharp
{
	public class TrpcTools
	{
		public static byte[] StringToUtf8Buffer(string input)
		{
			return Encoding.UTF8.GetBytes(input);
		}

		public static string Utf8BufferToString(byte[] buffer)
		{
			return Encoding.UTF8.GetString(buffer);
		}

		public static IntPtr StringToUtf8BufferPtr(string input)
		{
            return BytesToIntptr(StringToUtf8Buffer(input));
		}

        public static string Utf8BufferPtrToString(IntPtr ptr,int length)
        {
            if (ptr == IntPtr.Zero || length <= 0)
            {
                return "";
            }
            byte[] buf = IntptrToBytes(ptr, length);
            return Utf8BufferToString(buf);
        }

        //struct转换为byte[]
        public static byte[] StructToBytes(object structObj)
        {
            int size = Marshal.SizeOf(structObj);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(structObj, buffer, false);
            byte[] bytes = new byte[size];
            Marshal.Copy(buffer, bytes, 0, size);
            return bytes;
        }

        //byte[]转换为struct
        public static object BytesToStruct(byte[] bytes, Type type)
        {
            int size = Marshal.SizeOf(type);
            IntPtr buffer = Marshal.AllocHGlobal(size);
            Marshal.Copy(bytes, 0, buffer, size);
            return Marshal.PtrToStructure(buffer, type);
        }

        public static IntPtr BytesToIntptr(byte[] bytes)
        {
            int size = bytes.Length;
            IntPtr buffer = Marshal.AllocHGlobal(size);
            Marshal.Copy(bytes, 0, buffer, size);
            return buffer;
        }

        public static IntPtr StructureToPtr<T>(T info,bool fDeleteOld)
        {
            int size = Marshal.SizeOf(info);
            IntPtr intPtr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr<T>(info, intPtr, fDeleteOld);
            return intPtr;
        }

        public static byte[] IntptrToBytes(IntPtr ptr, int length)
        {
            byte[] ys = new byte[length];
            Marshal.Copy(ptr, ys, 0, length);
            return ys;
        }
    }
}

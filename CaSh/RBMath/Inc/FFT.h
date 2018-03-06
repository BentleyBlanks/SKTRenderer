#pragma once
#include "Vector2.h"
#include <iostream>

//CPUFFT
class RBFFTTools
{
public:
	static void iterative_fft(RBVector2 *v,RBVector2 *out,int len)
	{
		bit_reverse_copy(v,out,len);
		int n = len;
		for (int s = 1; s <= RBMath::log_2(n);s++)
		{
			int  m = RBMath::pow(2, s);
			RBVector2 wm(RBMath::cos(2 * PI / m), RBMath::sin(2 * PI / m));
			for (int k = 0; k < n ;k+=m)
			{
				RBVector2 w = RBVector2(1,0);
				for (int j = 0; j < m / 2;++j)
				{
					RBVector2 vv = out[k + j + m / 2];
					RBVector2 t = RBVector2(w.x*vv.x - w.y*vv.y,w.x*vv.y+w.y*vv.x);
					RBVector2 u = out[k + j];
					out[k + j] = RBVector2(u.x+t.x,u.y+t.y);
					out[k + j + m / 2] = RBVector2(u.x-t.x,u.y-t.y);
					w = RBVector2(w.x*wm.x - w.y*wm.y, w.x*wm.y + w.y*wm.x);
				}
			}
		}
	}

	static void iterative_ifft(RBVector2 *v, RBVector2 *out, int len)
	{
		bit_reverse_copy(v, out, len);
		int n = len;
		for (int s = 1; s <= RBMath::log_2(n); s++)
		{
			int  m = RBMath::pow(2, s);
			RBVector2 wm(RBMath::cos(2 * PI / m), RBMath::sin(2 * PI / m));
			for (int k = 0; k < n; k += m)
			{
				RBVector2 w = RBVector2(1, 0);
				for (int j = 0; j < m / 2; ++j)
				{
					RBVector2 vv = out[k + j + m / 2];
					RBVector2 t = RBVector2(w.x*vv.x - w.y*vv.y, w.x*vv.y + w.y*vv.x);
					RBVector2 u = out[k + j];
					out[k + j] = RBVector2(u.x + t.x, u.y + t.y);
					out[k + j + m / 2] = RBVector2(u.x - t.x, u.y - t.y);
					w = RBVector2(w.x*wm.x + w.y*wm.y, -w.x*wm.y + w.y*wm.x);
				}
			}
		}

		for (int i = 0; i < n;++i)
		{
			out[i].x /= n;
			out[i].y /= n;
		}

	}

	static void iterative_fft_2d(RBVector2 *source_2d, RBVector2 *source_2d_1, int N)
	{
		//C2DFFT
		//被执行2DFFT的是一个N*N的矩阵，在source_2d中按行顺序储存
		RBVector2* temp = new RBVector2[N*N];
		for (int i = 0; i < N*N; i++)
		{
			temp[i] = source_2d_1[i] = source_2d[i];
		}
		//水平方向FFT
		for (int i = 0; i < N; i++)
		{
			iterative_fft(&source_2d_1[i*N], &source_2d[i*N], N);
		}
		//转置列成行
		for (int i = 0; i < N*N; i++)
		{
			int x = i%N;
			int y = i / N;
			int index = x*N + y;
			source_2d_1[index] = source_2d[i];
		}
		//垂直FFT
		for (int i = 0; i < N; i++)
		{
			iterative_fft(&source_2d_1[i*N], &source_2d[i*N], N);
		}
		//转置回来
		for (int i = 0; i < N*N; i++)
		{
			int x = i%N;
			int y = i / N;
			int index = x*N + y;
			source_2d_1[index] = source_2d[i];
		}
		for (int i = 0; i < N*N; i++)
		{
			source_2d[i] = temp[i];
		}
		delete [] temp;
	}

	static void iterative_ifft_2d(RBVector2 *source_2d, RBVector2 *source_2d_1, int N)
	{
		//C2DFFT
		//被执行2DFFT的是一个N*N的矩阵，在source_2d中按行顺序储存
		RBVector2* temp = new RBVector2[N*N];
		for (int i = 0; i < N*N; i++)
		{
			temp[i] = source_2d_1[i] = source_2d[i];
		}
		//水平方向IFFT
		for (int i = 0; i < N; i++)
		{
			iterative_ifft(&source_2d_1[i*N], &source_2d[i*N], N);
		}
		//转置列成行
		for (int i = 0; i < N*N; i++)
		{
			int x = i%N;
			int y = i / N;
			int index = x*N + y;
			source_2d_1[index] = source_2d[i];
		}
		//垂直IFFT
		for (int i = 0; i < N; i++)
		{
			iterative_ifft(&source_2d_1[i*N], &source_2d[i*N], N);
		}
		//转置回来
		for (int i = 0; i < N*N; i++)
		{
			int x = i%N;
			int y = i / N;
			int index = x*N + y;
			source_2d_1[index] = source_2d[i];
		}
		for (int i = 0; i < N*N; i++)
		{
			source_2d[i] = temp[i];
		}
		delete temp;
	}
private:
	static INLINE void bit_reverse_copy(RBVector2 src[], RBVector2 des[], int len)
	{
		for (int i = 0; i < len;i++)
		{
			des[bit_rev(i, len-1)] = src[i];

		}
	}

public:
	static INLINE unsigned int bit_rev(unsigned int v, unsigned int maxv)
	{
		unsigned int t = RBMath::log_2(maxv + 1);
		unsigned int ret = 0;
		unsigned int s = 0x80000000>>(31);
		for (unsigned int i = 0; i < t; ++i)
		{
			unsigned int r = v&(s << i);
			ret |= (r << (t-i-1)) >> (i);	
		}
		return ret;
	}
};
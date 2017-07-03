// Test of matrix multiplication using Intel SSE instructions.
// These single instruction multiple data instructions allow
// for much faster multiplication.
// Runs on Intel x86 architectures only. I've only tested on mingw on Windows, I think it would
// require slight changes to the memory allocation routines to run on Mac/Linux
// 

#include<stdio.h>
#include<malloc.h>
#include <time.h>
#include <xmmintrin.h>
#include<math.h>

// some functions to make and check matrix multiplications
// n.b. this just makes a single matrix with numbers in, and an 
// identity matrix - it isn't a very good matrix test
void makeMatrices( float*a,float *b,const int DIM)
{
	for(int c=0;c<DIM;c++)
	{
		a[c+c*DIM]=c;
		a[c*DIM]=c;
		a[c]=c;
		b[c+c*DIM]=1; // b = identity matrix
	}
}

void checkMatrix(float *a,float *c,const int DIM)
{
	for(int i=0;i<DIM*DIM;i++)
	{
		if(fabs(a[i]-c[i])>0.001)
		{
			printf("Error in multiply %f %f\n",a[i],c[i]);
			return;
		}
	}
}

int main()
{
	
	clock_t start, end;
	const int DIM=1024;
	// these matrices need to be aligned in memory or else the SIMD stuff won't work
	// without modifying it to use unaligned load and store functions
	float *a=(float*)_aligned_malloc(DIM*DIM*sizeof(float),64);
	float *b=(float*)_aligned_malloc(DIM*DIM*sizeof(float),64);
	float *c=(float*)_aligned_malloc(DIM*DIM*sizeof(float),64);
	makeMatrices(a,b,DIM);
	printf("go\n");

	double cpu_time_used;

	// basic matrix multiply without any SIMD cleverness
	start = clock();
	for(int row=0;row<DIM;row++)
	{
	  for(int col=0;col<DIM;col++)
	  {
		c[row+col*DIM]=0;
		for(int cell=0;cell<DIM;cell++)
		{
			c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
		}
	  }
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done standard matrix multiply: %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);
	start = clock();
	// this version is unrolled over the output cells so it does
	// 4 rows at a time - exploiting in-order execution
	// faster than above but still not very fast (see test-loopunroll.cpp for more
	// of this kind of test)
	for(int row=0;row<DIM;row+=4)
	{
	  for(int col=0;col<DIM;col++)
	  {
		c[row+col*DIM]=0;
		c[row+1+col*DIM]=0;
		c[row+2+col*DIM]=0;
		c[row+3+col*DIM]=0;
		
		for(int cell=0;cell<DIM;cell++)
		{
			c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
			c[row+1+col*DIM]+=a[row+1+cell*DIM]*b[cell+col*DIM];
			c[row+2+col*DIM]+=a[row+2+cell*DIM]*b[cell+col*DIM];
			c[row+3+col*DIM]+=a[row+3+cell*DIM]*b[cell+col*DIM];
		}
	  }
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done unrolled matrix multiply: %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);
	start = clock();
	// this version uses simd to do 4 output cells at a time
	// which is significantly faster than normal
	__m128 zeroVector=_mm_setr_ps(0,0,0,0);
	for(int row=0;row<DIM;row+=4)
	{
	  for(int col=0;col<DIM;col++)
	  {
        __m128 sum= zeroVector;	
		for(int cell=0;cell<DIM;cell++)
		{
			__m128 aVal=_mm_load_ps(&a[row+cell*DIM]); // load 4 values from a
			__m128 bVal=_mm_load1_ps(&b[cell + col*DIM]); // load 1 value from b
			__m128 sumVal=_mm_mul_ps(aVal,bVal); // multiply 4 float values
			sum=_mm_add_ps(sum,sumVal); // add to the sum
		}
		_mm_store_ps(&c[row+col*DIM],sum);
	  }
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done SSE matrix multiply: %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);
	
	_aligned_free(a);
	_aligned_free(b);
	_aligned_free(c);
	
	return 0;
}
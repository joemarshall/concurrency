#include<stdio.h>
#include <time.h>
#include<math.h>
#include"threads.h"
const int DIM=1024;
const int NUM_THREADS=8;

float*a;
float *b;
float *c;

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


void multiplyThread(int colStart,int colEnd)
{
	if(colEnd>DIM)
	{
		colEnd=DIM;
	}
	for(int col=colStart;col<colEnd;col++)
	{
		for(int row=0;row<DIM;row++)
		{
			c[row+col*DIM]=0;
			for(int cell=0;cell<DIM;cell++)
			{
				c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
			}			
		}
	}
}

int main()
{
	clock_t start, end;
	a=new float[DIM*DIM];
	b=new float[DIM*DIM];
	c=new float[DIM*DIM];
	makeMatrices(a,b,DIM);
	printf("go\n");
	double cpu_time_used;

	// basic matrix multiply
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
	printf("done standard version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);	
	std::thread **threads=new std::thread*[NUM_THREADS];
	// threaded multiply
	start = clock();
	int start_col=0;
	int end_col=0;
	for(int c=0;c<NUM_THREADS;c++)
	{
		end_col=((c+1)*DIM)/NUM_THREADS;
		// launch a thread to do just some of the columns
		std::thread *t =new std::thread(std::bind(multiplyThread,start_col,end_col));
		//std::thread *t =new std::thread(multiplyThread,start_col,end_col,a,b,c); 
		threads[c]=t;
		start_col=end_col;
	}
	// wait for all threads to finish
	for(int c=0;c<NUM_THREADS;c++)
	{
		threads[c]->join();
	}
	// done
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done threaded multiply on %d threads %f\n",NUM_THREADS,cpu_time_used);
	checkMatrix(a,c,DIM);	
	delete a;
	delete b;
	delete c;
}

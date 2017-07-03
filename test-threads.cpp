// test of matrix multiply with threads

#include<stdio.h>
#include <time.h>
#include<math.h>
// this is for mingw, for other compilers you may nead #include<thread> instead
#include"threads.h"
const int DIM=1024;
const int NUM_THREADS=8;

float*a;
float *b;
float *c;

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


// this thread function just calculates the output value for all cells in a 
// range of columns multiple threads are made to do the 
// full matrix multiply
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

	// basic matrix multiply - in the single main thread
	start = clock();
	// cycle round columns and rows of the output matrix 
	for(int row=0;row<DIM;row++)
	{
	  for(int col=0;col<DIM;col++)
	  {
		// calculate one output cell
		c[row+col*DIM]=0;
		for(int cell=0;cell<DIM;cell++)
		{
			c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
		}
	  }
	}
	end = clock();
	// time how long it takes
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done standard version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);	
	std::thread **threads=new std::thread*[NUM_THREADS];
	// threaded multiply
	start = clock();
	int start_col=0;
	int end_col=0;
	// multiply matrix in thread - this launches all the threads
	// each of which calculates a subset of the columns of the 
	// output matrix
	for(int c=0;c<NUM_THREADS;c++)
	{
		end_col=((c+1)*DIM)/NUM_THREADS;
		// launch a thread to do just some of the columns
		std::thread *t =new std::thread(std::bind(multiplyThread,start_col,end_col));
		threads[c]=t;
		start_col=end_col;
	}
	// then we wait for all threads to finish
	for(int c=0;c<NUM_THREADS;c++)
	{
		threads[c]->join();
	}
	// once all threads are finished, we are done
	end = clock();
	// time how long it takes
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done threaded multiply on %d threads %f\n",NUM_THREADS,cpu_time_used);
	checkMatrix(a,c,DIM);	
	// don't forget to delete the allocated memory
	delete a;
	delete b;
	delete c;
}

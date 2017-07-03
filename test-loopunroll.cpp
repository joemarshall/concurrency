// tests of matrix multiplication demonstrating differences in performance when
// we do multiple non-dependent calculations per loop (due to out of order execution)

#include<stdio.h>
#include <time.h>
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
	// make some matrix buffers
	float *a=new float[DIM*DIM];
	float *b=new float[DIM*DIM];
	float *c=new float[DIM*DIM];
	
	printf("go\n");

	double cpu_time_used;

	// basic matrix multiply
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
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	// time how long it takes
	printf("done standard version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);	
	start = clock();
	// this version is 'unrolled' over the output cells so it does 4 at a time
	// this is significantly faster because it can easily be vectorized or
	// the instructions can be executed out of order
	// (what it does actually depends on your compiler settings)
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
	printf("done output cell unrolled version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);

	// this version is unrolled so it does 4 cell steps at a time 
	// this is not faster because each calculation within the cell step 
	// (the adding to the sum) depends on the previous one
	// so it can't be vectorized or out of order executed
	start = clock();
	for(int row=0;row<DIM;row++)
	{
	  for(int col=0;col<DIM;col++)
	  {
		c[row+col*DIM]=0;
		for(int cell=0;cell<DIM;cell+=4)
		{
			c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
			c[row+col*DIM]+=a[row+(cell+1)*DIM]*b[cell+1+col*DIM];
			c[row+col*DIM]+=a[row+(cell+2)*DIM]*b[cell+2+col*DIM];
			c[row+col*DIM]+=a[row+(cell+3)*DIM]*b[cell+3+col*DIM];
		}
	  }
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done cell step version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);

	
	// don't forget to delete the allocated memory
	delete a;
	delete b;
	delete c;
	return 0;
}
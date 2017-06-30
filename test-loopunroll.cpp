#include<stdio.h>
#include <time.h>
#include<math.h>

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
	float *a=new float[DIM*DIM];
	float *b=new float[DIM*DIM];
	float *c=new float[DIM*DIM];
	
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
	start = clock();
	// this version is unrolled over the output cells so it does 4 at a time
	// significantly faster
	for(int row=0;row<DIM;row++)
	{
	  for(int col=0;col<DIM;col+=4)
	  {
		c[row+col*DIM]=0;
		c[row+col*DIM+1]=0;
		c[row+col*DIM+2]=0;
		c[row+col*DIM+3]=0;
		
		for(int cell=0;cell<DIM;cell++)
		{
			c[row+col*DIM]+=a[row+cell*DIM]*b[cell+col*DIM];
			c[row+col*DIM+1]+=a[row+cell*DIM+1]*b[cell+col*DIM+1];
			c[row+col*DIM+2]+=a[row+cell*DIM+2]*b[cell+col*DIM+2];
			c[row+col*DIM+3]+=a[row+cell*DIM+3]*b[cell+col*DIM+3];
		}
	  }
	}
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("done output cell unrolled version %f\n",cpu_time_used);
	checkMatrix(a,c,DIM);
	// this version is unrolled so it does 4 cell steps at a time - not faster
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

	
	delete a;
	delete b;
	delete c;
	return 0;
}
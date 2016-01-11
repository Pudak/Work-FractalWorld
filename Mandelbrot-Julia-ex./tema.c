#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <math.h>


int main(int argc, char** argv) {

    int rank, size;
    MPI_Status status;   

    // init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    MPI_Barrier(MPI_COMM_WORLD);

    
    if(rank == 0)  
    {
		int type,iteration[3];
		double xmin,xmax,ymin,ymax,   resolution;
		double cx,cy;
		int tmp;
		int i, j;

		//citesc datele
		FILE *fin  = fopen(argv[1],"r");

		tmp = fscanf(fin,"%d\n",&type);
		tmp = fscanf(fin,"%lf %lf %lf %lf\n",&xmin,&xmax,&ymin,&ymax);
		tmp = fscanf(fin,"%lf\n",&resolution);
		tmp = fscanf(fin,"%d\n",&iteration[0]);
		iteration[1] = type;

		double vectorFloat[7];//this is a vector which is sent to other clients

		if(type == 1){//for julia
			tmp = fscanf(fin,"%lf %lf\n",&cx,&cy);
			vectorFloat[5] = cx;
			vectorFloat[6] = cy;
 		}

		vectorFloat[0] = xmin;	vectorFloat[1] = xmax;	vectorFloat[2] = ymin;	vectorFloat[3] = ymax;	vectorFloat[4] = resolution;

		//send to other clients except root
		for(i=1;i<size;i++)
		{
			MPI_Send(&vectorFloat, 7, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
			MPI_Send(&iteration, 2, MPI_INT, i, 2, MPI_COMM_WORLD); //contains [0] iteration, [1] type
		}
		


		int width = (int)((xmax-xmin)/resolution);
		int height = (int)((ymax-ymin)/resolution);

		
		FILE *fout  = fopen(argv[2],"w");
		fprintf(fout,"P2\n");
		fprintf(fout,"%d %d\n",width,height);
		fprintf(fout,"255\n");

		//allocaation for matrix
		int **mat = (int **)malloc(width * sizeof(int*)); //matrix dynamic
		for(i = 0; i < width; i++) mat[i] = (int *)malloc((height) * sizeof(int));
		
		double zx,zy,zx2,zy2 ;
		int it;
		
		//this is for root part. if it is mandelbrot
		if(type == 0)
		{
			for(j=0; j<height/size; j++)
			{
				cy=ymin + (double)j*resolution;
				for(i=0; i<width;  i++)
				{

					cx=xmin + (double)i*resolution;
					zx=0.0f;
					zy=0.0f;
					zx2=zx*zx;
					zy2=zy*zy;
					
					for(it=0;it<iteration[0] && zx2+zy2<4.0f; it++)
					{
						zy = 2.0f*zx*zy + cy;
						zx = zx2-zy2 + cx;
						zx2 = zx*zx;
						zy2 = zy*zy;
					}
					mat[i][j] = it%256;
				}
			}
		}else if(type == 1)
		{
			for(j=0; j<height/size; j++)
			{

				for(i=0; i<width;  i++)
				{
					zx=xmin + ((double)i)*resolution;
					zy=ymin + ((double)j)*resolution;
					
					zx2=zx*zx;
					zy2=zy*zy;
					
					for(it=0;it<iteration[0] && zx2+zy2<4.0f; it++)
					{
						zy = 2.0f*zx*zy + cy;
						zx = zx2-zy2 + cx;
						zx2 = zx*zx;
						zy2 = zy*zy;
					}
					mat[i][j] = (it)%256;
				}
			}
		}
		
		//this vec for collecting datas from each clients with the size of height/size. I divided only height which is j
		int *vec = (int *)malloc(width * (height/size) * sizeof(int));
		int offset ;
		int k;
		
		//recieve from each clients other than root with tag 3
		for(k=1;k<size;k++){
			MPI_Recv(vec, width * (height/size), MPI_INT, k, 3, MPI_COMM_WORLD, &status);
			
			for(j = (height/size)*(k); j<(height/size)*(k+1); j++)
			{
				for(i=0; i<width;  i++)
				{
					offset = i + width * (j-(height/size)*(k));
					mat[i][j] = vec[offset];
				}
			}
		}


		for(j=height-1;j>=0;j--)
		{
			for(i=0;i<width;i++)
				fprintf(fout,"%d ",mat[i][j]);
			fprintf(fout,"\n");
		}

		fclose(fin);
		fclose(fout);


    }

	if(rank != 0)
	{
		double vectorFloat2[7];
		int iter[3];
		int i, j;
		double cx,cy;

		MPI_Recv(&vectorFloat2, 7, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&iter, 2, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

		int width = (int)((vectorFloat2[1]-vectorFloat2[0])/vectorFloat2[4]);
		int height = (int)((vectorFloat2[3]-vectorFloat2[2])/vectorFloat2[4]);
		
		int *mat = (int *)malloc(width * (height/size) * sizeof(int));
		int offset ;//= i + rows * j;
		
		double zx,zy,zx2,zy2 ;
		int it;
		
	//similar as root. but this ones are for clients. keeps datas only in a vector. which i can use instead of matrix
	if(iter[1] == 0)
	{
		for(j=(height/size)*(rank); j<(height/size)*(rank+1); j++)
		{
			cy=vectorFloat2[2] + ((double)j)*vectorFloat2[4];

			for(i=0; i<width;  i++)
			{

				cx=vectorFloat2[0] + (double)i*vectorFloat2[4];
				zx=0.0f;
				zy=0.0f;
				zx2=zx*zx;
				zy2=zy*zy;
				
				for(it=0;it<iter[0] && zx2+zy2<4.0f; it++)
				{
					zy = 2.0f*zx*zy + cy;
					zx = zx2-zy2 + cx;
					zx2 = zx*zx;
					zy2 = zy*zy;
				}

				offset = i + width * (j-(height/size)*(rank));
				mat[offset] = it%256;
			}
		}
	}//endif
	else if(iter[1] == 1)
	{
		cx = vectorFloat2[5];
		cy = vectorFloat2[6];

			for(j=(height/size)*rank; j<(height/size)*(rank+1); j++)
			{

				for(i=0; i<width;  i++)
				{
					zx=vectorFloat2[0] + ((double)i)*vectorFloat2[4];
					zy=vectorFloat2[2] + ((double)j)*vectorFloat2[4];
					
					zx2=zx*zx;
					zy2=zy*zy;
					
					for(it=0;it<iter[0] && zx2+zy2<4.0f; it++)
					{
						zy = 2.0f*zx*zy + cy;
						zx = zx2-zy2 + cx;
						zx2 = zx*zx;
						zy2 = zy*zy;
					}
					offset = i + width * (j-(height/size)*(rank));
					mat[offset] = it%256;				
				}
			}
	}

		//sends back to the root for final printing
		MPI_Send(mat, width*(height/size), MPI_INT, 0, 3, MPI_COMM_WORLD);

	}

	
	MPI_Finalize();	   
}
    


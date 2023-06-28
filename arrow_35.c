#include <stdlib.h>
#include <stdio.h> /*printf*/
#include <math.h>
#include <stdint.h>
#include <omp.h> /*openmp*/
#include <string.h> /*mcpy*/



uint64_t expo(uint64_t x, uint64_t n){
    uint64_t temp;
    if (n==0){return 1;}
    else{
        if (n==1){return x;}
        else{
            temp = expo(x,n/2);
            if (n%2 == 0){return temp*temp;}
            else{return x*temp*temp;}
            }
        }
    }
        
void initialisation(uint64_t* x, uint64_t* y, uint64_t* w, uint64_t s, uint64_t m){
/*step 1: create the internal states
(warning ! because of rand the internal states are always the same) */
    uint64_t i;
    uint64_t j;
    uint64_t temp_x;
    uint64_t temp_y;
    uint64_t MASK;
    
    MASK = expo(2,4*s)-1;
    
    for (i=0;i<5;i++){
        x[i] = rand() & MASK;
        printf("%li, ",x[i]);
        }   
        printf("\n");
    for (i=0;i<3;i++){
        y[i] = rand() & MASK;
        printf("%li, ",y[i]);
        }
        printf("\n");
/*step 2: run the PRNG
(warning ! we erased the original internal states) */
    for(i=0;i<m;i++){
        temp_x = ( (x[4]^(y[0]<<s)) + (x[1]^(y[2]>>s)) ) & MASK;
        temp_y = ( (y[2]^(x[1]<<s)) + (y[0]^(x[4]>>s)) ) & MASK;
        w[i] = temp_x^temp_y;
        
        for (j=4;j>0;j--){
            x[j] = x[j-1];
            }
        x[0] = temp_x;
        
        for (j=2;j>0;j--){
            y[j] = y[j-1];
        }
        y[0] = temp_y;
    }
    return;
}        
        
void copy(uint64_t* dep, uint64_t* arr, int size){
    memcpy((void*) arr, (void*) dep, size*sizeof(uint64_t)); 
    } 
      
void clock(uint64_t t, uint64_t* up, uint64_t* down, uint64_t* w_1, uint64_t* w_2,uint64_t* w_3, uint64_t* w_4, uint64_t x_1, uint64_t x_2, uint64_t x_3, uint64_t x_4, uint64_t y_1, uint64_t y_2, uint64_t y_3, uint64_t y_4){
/*we modify the list, so we have to deep copy if needed*/  
    uint64_t i;
    uint64_t temp[16];
    
    /*Mirror*/
    up[0] = down[0]^w_1[t-1];
    up[1] = down[1]^w_2[t-1];
    up[2] = down[2]^w_3[t-1];
    up[3] = down[3]^w_4[t-1];
    
    down[4] = up[4]^w_1[t-2];
    down[5] = up[5]^w_2[t-2];
    down[6] = up[6]^w_3[t-2];
    down[7] = up[7]^w_4[t-2];
    
    up[8] = down[8]^w_1[t-3];
    up[9] = down[9]^w_2[t-3];
    up[10] = down[10]^w_3[t-3];
    up[11] = down[11]^w_4[t-3];

    /*shift*/
    
    copy(up,temp,16);
    copy(temp,up+4,16);
    up[3] = x_4;
    up[2] = x_3;
    up[1] = x_2;
    up[0] = x_1;
    
    /*for(i=13;n0>1;i--) down[i] = down[i-2];*/
    copy(down,temp,8);
    copy(temp,down+4,8);
    down[3] = y_4;
    down[2] = y_3;
    down[1] = y_2;
    down[0] = y_1;
    return;
}  


void save(uint64_t* up, uint64_t* down, uint64_t* up0, uint64_t* down0){
    copy(up, up0, 20);
    copy( down, down0, 12);
    }
    
uint64_t check(uint64_t s,uint64_t* up1,uint64_t* down1,uint64_t t,uint64_t* w_1,uint64_t* w_2, uint64_t* w_3, uint64_t* w_4, uint64_t nb_round, uint64_t mask){
    uint64_t up[20];
    uint64_t down[12];
    uint64_t n0;
    uint64_t a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
    uint64_t x_1,x_2,x_3,x_4,y_1,y_2,y_3,y_4,rx1,rx2,rx3,ry1,ry2,ry3;

    /*printf("mask %i \n",mask);*/
    
    save(up1,down1,up,down);

    for(n0=0;n0<nb_round;n0++){
        a = up[19];                                                      
        b = up[18];
        c = up[17];
        d = up[16];
        e = up[7];
        f = up[6];
        g = up[5];
        h = up[4];
        i = down[11];
        j = down[10];
        k = down[9];
        l = down[8];
        m = down[3];
        n = down[2];
        o = down[1];
        p = down[0];
                                                                        
        x_1 = (d+(h^k)) & mask;
        y_1 = (l+(c^p)) & mask;                                                            
        rx1 = (d+(h^k))>> s;
        ry1 = (l+(c^p))>> s;
        
        x_2 = ((c^p)+(g^j)+rx1) & mask;
        y_2 = ((h^k)+(b^o)+ry1)& mask;
        rx2 = ((c^p)+(g^j)+rx1)>> s;
        ry2 = ((h^k)+(b^o)+ry1)>> s;
        
        x_3 = ((b^o)+(f^i)+rx2)& mask;
        y_3 = ((g^j)+(a^n)+ry2)& mask;
        rx3 = ((b^o)+(f^i)+rx2)>> s;
        ry3 = ((g^j)+(a^n)+ry2)>> s;
        
        x_4 = ((a^n)+e+rx3)& mask;
        y_4 = ((f^i)+m+ry3)& mask;                             
                                                                        
        if(((x_1^y_1) == w_1[t+n0]) && ((x_2^y_2) == w_2[t+n0]) && ((x_3^y_3) == w_3[t+n0]) && ((x_4^y_4) == w_4[t+n0])){
            clock(t+n0,up,down,w_1,w_2,w_3,w_4,x_1,x_2,x_3,x_4,y_1,y_2,y_3,y_4);
            }
        else return 0;
        }
    return 1;                
    }
    
uint64_t attack_arrow(uint64_t* w, uint64_t s,uint64_t m){
    uint64_t res = 0;
    uint64_t w_1[m], w_2[m],w_3[m],w_4[m];
    uint64_t n0;
    uint64_t mask = expo(2,s)-1;
    
    for(n0=0; n0<m; n0++){
        w_1[n0] = w[n0] & mask;
        w_2[n0] = (w[n0]>>s) & mask;
        w_3[n0] = (w[n0]>>(2*s)) & mask;
        w_4[n0] = (w[n0]>>(3*s)) & mask;
    }
    #pragma omp parallel for
    for(n0=0; n0< expo(2,12*s);n0++){
        uint64_t up[20];
        uint64_t down[12];
        uint64_t a3 = n0 & mask;
        up[19] = a3;
        uint64_t b3 = (n0>>s) & mask;
        up[18] = b3;
        uint64_t c3 = (n0>>2*s) & mask;
        up[17] = c3;
        uint64_t d3 = (n0>>3*s) & mask;
        up[16] = d3;
        
        uint64_t i3 = (n0>>4*s) & mask;
        down[11]  = i3;
        uint64_t j3 = (n0>>5*s) & mask;
        down[10] = j3;
        uint64_t k3 = (n0>>6*s) & mask;
        down[9] = k3;
        uint64_t l3 = (n0>>7*s) & mask;
        down[8] = l3;
        
        uint64_t e3 = (n0>>8*s) & mask;
        up[7] = e3;
        uint64_t f3 = (n0>>9*s) & mask;
        up[6] = f3;
        uint64_t g3 = (n0>>10*s) & mask;
        up[5] = g3;
        uint64_t h3 = (n0>>11*s) & mask;
        up[4] = h3;

        uint64_t x_1 = (d3+(h3^k3)) & mask;
        uint64_t rx1 = (d3+(h3^k3))>> s;
        uint64_t y_1 = w_1[3]^x_1;
        uint64_t p3 = ((y_1-l3)^c3) & mask;
        down[0] = p3;                          
        uint64_t ry1 = (l3+(c3^p3))>> s;
        
        uint64_t x_2 = ((c3^p3)+(g3^j3)+rx1) & mask;
        uint64_t rx2 = ((c3^p3)+(g3^j3)+rx1)>> s;
        uint64_t y_2 = w_2[3]^x_2;
        uint64_t o3 = ((y_2-(h3^k3)-ry1)^b3) & mask;
        down[1] = o3;
        uint64_t ry2 = ((h3^k3)+(b3^o3)+ry1)>> s;
        
        uint64_t x_3 = ((b3^o3)+(f3^i3)+rx2)& mask;
        uint64_t rx3 = ((b3^o3)+(f3^i3)+rx2)>> s;
        uint64_t y_3 = w_3[3]^x_3;
        uint64_t n3 = ((y_3-(g3^j3)-ry2)^a3) & mask;
        down[2] = n3;
        uint64_t ry3 = ((g3^j3)+(a3^n3)+ry2)>> s;
        
        uint64_t x_4 = ((a3^n3)+e3+rx3)& mask;
        uint64_t y_4 = w_4[3]^x_4;
        uint64_t m3 = (y_4-(f3^i3)-ry3) & mask;
        down[3] = m3;
        
        clock(3,up,down,w_1,w_2,w_3,w_4,x_1,x_2,x_3,x_4,y_1,y_2,y_3,y_4);
        clock(4,up,down,w_1,w_2,w_3,w_4,0,0,0,0,0,0,0,0);

        uint64_t a5 = up[19];
        uint64_t b5 = up[18];
        uint64_t c5 = up[17];
        uint64_t d5 = up[16];
        
        uint64_t i5 = down[11];
        uint64_t j5 = down[10];
        uint64_t k5 = down[9];
        uint64_t l5 = down[8];

        uint64_t e5 = up[7];
        uint64_t f5 = up[6];
        uint64_t g5 = up[5];
        uint64_t h5 = up[4];

        x_1 = (d5+(h5^k5)) & mask;
        rx1 = (d5+(h5^k5))>> s;
        y_1 = w_1[5]^x_1;
        uint64_t p5 = ((y_1-l5)^c5) & mask;
        down[0] = p5;                          
        ry1 = (l5+(c5^p5))>> s;
        
        x_2 = ((c5^p5)+(g5^j5)+rx1) & mask;
        rx2 = ((c5^p5)+(g5^j5)+rx1)>> s;
        y_2 = w_2[5]^x_2;
        uint64_t o5 = ((y_2-(h5^k5)-ry1)^b5) & mask;
        down[1] = o5;
        ry2 = ((h5^k5)+(b5^o5)+ry1)>> s;
        
        x_3 = ((b5^o5)+(f5^i5)+rx2)& mask;
        rx3 = ((b5^o5)+(f5^i5)+rx2)>> s;
        y_3 = w_3[5]^x_3;
        uint64_t n5 = ((y_3-(g5^j5)-ry2)^a5) & mask;
        down[2] = n5;
        ry3 = ((g5^j5)+(a5^n5)+ry2)>> s;
        
        x_4 = ((a5^n5)+e5+rx3)& mask;
        y_4 = w_4[5]^x_4;
        uint64_t m5 = (y_4-(f5^i5)-ry3) & mask;
        down[3] = m5;
        clock(5,up,down,w_1,w_2,w_3,w_4,x_1,x_2,x_3,x_4,y_1,y_2,y_3,y_4);
        if(check(s,up,down,6,w_1,w_2,w_3,w_4,4,mask)){
            res++ ;
        }
    }
    return res;
}

int main(){
    uint64_t x[5];
    uint64_t y[3];
    uint64_t w[10];

    uint64_t s;
    uint64_t m;

    
    s=2;
    m=10;
    initialisation(x,y,w,s,m);
    uint64_t res = attack_arrow(w,s,m);
    printf("%li",res);
    return 0;
    }

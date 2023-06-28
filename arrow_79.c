#include <stdlib.h>
#include <stdio.h> /*printf*/
#include <math.h>
#include <stdint.h>
#include <omp.h> /*openmp*/
#include <string.h> /*mcpy*/
#include <time.h>


uint32_t expo(uint32_t x, uint32_t n){
    uint32_t temp;
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
        
void initialisation(uint32_t* x, uint32_t* y, uint32_t* w, uint32_t s){
/*step1: create the internal states
(warning! Because of rand we always have the same internal states) */
    uint32_t i;
    uint32_t j;
    uint32_t temp_x;
    uint32_t temp_y;
    uint32_t MASK;
    
    MASK = expo(2,2*s)-1;
    
    for (i=0;i<9;i++){
        x[i] = rand() & MASK;
        printf("%i, ",x[i]);
        }   
        printf("\n");
    for (i=0;i<7;i++){
        y[i] = rand() & MASK;
        printf("%i, ",y[i]);
        }
        printf("\n");
/*step 2: run the PRNG
(Warning! We erase the original initial states */
    printf("w = [");
    for(i=0;i<40;i++){
        temp_x = ( (x[8]^(y[2]<<s)) + (x[3]^(y[6]>>s)) ) & MASK;
        temp_y = ( (y[6]^(x[3]<<s)) + (y[2]^(x[8]>>s)) ) & MASK;
        w[i] = temp_x^temp_y;
        printf("%i ,",w[i]);
        
        for (j=8;j>0;j--){
            x[j] = x[j-1];
            }
        x[0] = temp_x;
        
        for (j=6;j>0;j--){
            y[j] = y[j-1];
        }
        y[0] = temp_y;
    }
    printf("] \n");
    return;
}

void copy(uint32_t* dep, uint32_t* arr, int size){
    memcpy((void*) arr, (void*) dep, size*sizeof(uint32_t)); 
    }  
      
void clockarrow(uint32_t t, uint32_t* up, uint32_t* down, uint32_t* w_l, uint32_t* w_h, uint32_t x_l, uint32_t x_h, uint32_t y_l, uint32_t y_h){
/* Warning, we modify the list so we must deep copy at some points*/   
    uint32_t i;
    uint32_t temp[16];
    
    /*Mirror*/
    up[4] = down[4] ^ w_l[t-3];
    up[5] = down[5] ^ w_h[t-3];
    down[6] = up[6] ^ w_l[t-4];
    down[7] = up[7] ^ w_h[t-4];
    up[12] = down[12] ^ w_l[t-7];
    up[13] = down[13] ^ w_h[t-7];

    /*shift*/
    
    copy(up,temp,16);
    copy(temp,up+2,16);
    /*for(i=17;i>1;i--) up[i] = up[i-2];*/
    up[1] = x_h;
    up[0] = x_l;
    
    /*for(i=13;i>1;i--) down[i] = down[i-2];*/
    copy(down,temp,12);
    copy(temp,down+2,12);
    down[1] = y_h;
    down[0] = y_l;
    return;
}

   

void save(uint32_t* up, uint32_t* down, uint32_t* up0, uint32_t* down0){
    copy(up, up0, 18);
    copy( down, down0, 14);
    }
    
uint32_t check(uint32_t s,uint32_t* up1,uint32_t* down1,uint32_t t,uint32_t* w_l,uint32_t* w_h, uint32_t nb_round, uint32_t mask){
    uint32_t up[18];
    uint32_t down[14];
    uint32_t i;
    uint32_t a,b,c,d,e,f,g,h;
    uint32_t x_l,y_l,x_h,y_h,rx,ry;

    
    save(up1,down1,up,down);

    for(i=0;i<nb_round;i++){
        a = up[17];                                                      
        b = up[16];
        c = up[7];
        d = up[6];
        e = down[13];
        f = down[12];
        g = down[5];
        h = down[4];
                                                                        
        x_l = (b+(e^d)) & mask;
        y_l = (f+(a^h)) & mask;        
        rx = (b+(e^d)) >> s;
        /*printf("rx %i \n", rx);*/
        ry = (f+(a^h)) >> s;
        /*printf("ry %i \n", ry);*/
        x_h = (c+(a^h)+rx) & mask;
        y_h = (g+(e^d)+ry) & mask;                                 
                                                                        
        if((x_l^y_l) == w_l[t+i] && (x_h^y_h) == w_h[t+i]){
            clockarrow(t+i,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
            }
        else return 0;
        }
    return 1;                
    }   
    
uint32_t attack(uint32_t* w, uint32_t s){
    uint32_t res = 0;
    uint32_t w_h[40], w_l[40];
    uint32_t n0;
    
    uint32_t j;
    uint32_t size_n0 = expo(2,4*s);
    uint32_t size_n1 = expo(2,3*s);
    uint32_t size_n2 = expo(2,s);
    uint32_t size_n3 = expo(2,s+2);
    uint32_t mask = expo(2,s)-1;
    
    double cpu_time_used;
    clock_t start, end;
    start = clock();
    
    for(j=0;j<40;j++){
        w_h[j] = w[j]>>s;
        w_l[j] = w[j] & mask;
    }
        
    #pragma omp parallel for
    for(n0=0;n0<size_n0;n0++){
        /*t=7*/
        uint32_t n1,n2,n3;
        uint32_t i;
        uint32_t up[18],down[14];
        uint32_t rx,ry,x_l,x_h,y_l,y_h;
        uint32_t up0[18],down0[14],up1[18],down1[14],up2[18],down2[14];
        
        uint32_t a_xor_h7 = n0 & mask;
        uint32_t e_xor_d7 = (n0 >> s) & mask;
        uint32_t b7 = (n0 >> (2*s)) & mask;
        uint32_t g7 = (n0 >> (3*s)) & mask;
        
        for(i=0;i<18;i++){
            up[i] = 0;
        }
        for(i=0;i<14;i++){
            down[i]=0;
        }
        up[16] = b7;
        down[5] = g7;
                    
        rx = (b7 + e_xor_d7) >> s;
        x_l = (b7 + e_xor_d7) & mask ;
        y_l = w_l[7]^x_l;
        uint32_t f7 = (y_l - a_xor_h7) & mask;
        down[12] = f7;

                    
        ry = (f7 + a_xor_h7) >> s;
        y_h = (e_xor_d7 + g7 + ry) & mask;
        x_h = w_h[7]^y_h;
        uint32_t c7 = (x_h - a_xor_h7 - rx) & mask;
        up[7] = c7;
                   
        clockarrow(7,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
        clockarrow(8,up,down,w_l,w_h,0,0,0,0);
        
        save(up,down,up0,down0);


        uint32_t b9 = up[16];
        uint32_t c11 = x_h;
        uint32_t d11 = x_l;
        uint32_t e11 = g7;
        uint32_t e_xor_d11 = e11^d11;
        uint32_t a12 = c7;
        uint32_t a16 = x_h;
        uint32_t b16 = x_l;
        
    for(n1=0;n1<size_n1;n1++){
        /*t= 9*/
        uint32_t a_xor_h9 = n1 & mask;
        uint32_t e_xor_d9 = (n1 >> s) & mask;
        uint32_t g9 = (n1 >> (2*s)) & mask;

        save(up0,down0,up,down);
                              
        down[5] = g9;
        rx = (b9 + e_xor_d9) >> s;
        x_l = (b9 + e_xor_d9) & mask;
        y_l = w_l[9]^x_l;
        uint32_t f9 = (y_l - a_xor_h9) & mask;
        down[12] = f9;
                    
        ry = (f9 + a_xor_h9) >> s;
        y_h = (e_xor_d9 + g9 + ry) & mask;
        x_h = w_h[9]^y_h;
        uint32_t c9 = (x_h - a_xor_h9 - rx) & mask;
        up[7] = c9;    
                                      
        clockarrow(9,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
        clockarrow(10,up,down,w_l,w_h,0,0,0,0);
        
        save(up,down,up1,down1);
        
        uint32_t b11 = up[16];
        uint32_t e12 = c9^w_h[5];
        uint32_t g12 = y_h;
        uint32_t h12 = y_l;    
        uint32_t a_xor_h12 = a12^h12;
        uint32_t a15 = g9^w_h[6];
        uint32_t e16 = y_h;
        uint32_t f16 = y_l;
        uint32_t a18 = x_h;
        uint32_t b18 = x_l;
          
    for(n2=0;n2<size_n2;n2++){
        /* t= 11*/
    
        uint32_t f11 = n2 ;

        save(up1,down1,up,down);    
        rx = (b11 + e_xor_d11) >>s ;
        x_l = (b11 + e_xor_d11) & mask  ;
        y_l = w_l[11]^x_l;
        uint32_t a_xor_h11 = (y_l - f11) & mask;
        ry = (f11 + a_xor_h11) >> s;
        x_h = (a_xor_h11+c11+rx) & mask;
        y_h = x_h^w_h[11];
        uint32_t g11 = (y_h- e_xor_d11 - ry) & mask;
        down[5] = g11;
        
                                
        clockarrow(11,up,down,w_l,w_h,x_l,x_h,y_l,y_h);    
        save(up,down,up2,down2);
        
        uint32_t c12 = up[7];
        uint32_t a_plus_c = a_xor_h12 + c12;
        uint32_t c15 = x_h;
        uint32_t d15 = x_l;
        uint32_t e15 = g11;
        uint32_t e_xor_d15 = e15^d15;
        uint32_t e18 = y_h;
        uint32_t f18 = y_l;
        uint32_t a20 = x_h;
        uint32_t b20 = x_l;
        
    for(n3=0;n3<size_n3;n3++){
        /* t=12 */
        uint32_t b12 = n3 & mask;
        rx = (n3 >> s) & 1;
        ry = (n3 >> (s+1)) & 1 ;      
        save(up2,down2,up,down);          
                                                 
        x_h = (a_plus_c +rx) & mask;
        y_h = x_h^w_h[12];
     
        uint32_t e_xor_d12 = (y_h - g12 - ry) & mask;
        uint32_t d12 = e_xor_d12^e12;
        up[6] = d12;
        x_l = (b12 + e_xor_d12) & mask;
        y_l = x_l^w_l[12];
        uint32_t f12 = (y_l - a_xor_h12) & mask ;    
             
        
        if (((b12+e_xor_d12) >> s  == rx)  && ((f12+a_xor_h12) >> s == ry)){
        
        
            down[12]= f12;
            clockarrow(12,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
            clockarrow(13,up,down,w_l,w_h,0,0,0,0);
            clockarrow(14,up,down,w_l,w_h,0,0,0,0);
            /*t=15*/                                  

            uint32_t f15 = down[12];
            uint32_t g15 = down[5];
            uint32_t h15 = down[4];              
            uint32_t a_xor_h15 = a15^h15;
                                     
            y_l = (f15+a_xor_h15) & mask;
            ry = (f15+a_xor_h15) >> s;
            x_l = w_l[15]^y_l;
            uint32_t b15 = (x_l - e_xor_d15) & mask;
            rx = (b15 +e_xor_d15) >> s;
            y_h = (g15+e_xor_d15+ry) & mask;
            x_h = (a_xor_h15 + c15 +rx) & mask ;   
            if((x_h^y_h) == w_h[15]){
                clockarrow(15,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
        
                
                /*t=16*/

                uint32_t c16 = up[7];
                uint32_t d16 = up[6];

                uint32_t e_xor_d16 = e16^d16;
                x_l = (e_xor_d16 + b16) & mask ;
                rx = (b16 + e_xor_d16) >> s;
                y_l = x_l^w_l[16];
                uint32_t a_xor_h16 = (y_l - f16) & mask ;
                uint32_t h16 = a16^a_xor_h16 ;
                down[4] = h16;
                ry = (a_xor_h16+f16) >> s;
                x_h = (a_xor_h16 + c16 + rx) & mask;
                y_h = x_h^w_h[16];
                uint32_t g16 = (y_h - e_xor_d16-ry) & mask;
                down[5] = g16;
                                                            
                clockarrow(16,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
                clockarrow(17,up,down,w_l,w_h,0,0,0,0);
                /*t=18*/
                                                            
                uint32_t g18 = down[5];
                uint32_t h18 = down[4];
                                                            
                uint32_t a_xor_h18 = a18^h18;
                y_l = (f18+a_xor_h18) & mask;
                ry = (f18+a_xor_h18) >> s;
                x_l = y_l^w_l[18];
                uint32_t e_xor_d18 = (x_l - b18) & mask;
                uint32_t d18=e18^e_xor_d18;
                up[6] = d18;
                rx = (b18+e_xor_d18) >> s;
                y_h = (e_xor_d18+g18+ry) & mask;
                x_h = y_h^w_h[18];
                uint32_t c18 = (x_h-a_xor_h18-rx) & mask;
                up[7] = c18;
                                                            
                clockarrow(18,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
                clockarrow(19,up,down,w_l,w_h,0,0,0,0);
                /*t=20*/
                                    
                uint32_t c20 = up[7];
                uint32_t d20 = up[6];
                uint32_t e20 = down[13];
                uint32_t f20 = down[12];
                     
                uint32_t e_xor_d20 = e20^d20;
                x_l = (b20+e_xor_d20) & mask;
                rx = (b20+e_xor_d20) >> s;
                y_l = x_l^w_l[20];
                uint32_t a_xor_h20 = (y_l-f20) & mask;
                ry = (f20+a_xor_h20) >> s;
                uint32_t h20 = a_xor_h20^a20;
                down[4] = h20;
                x_h = (c20+a_xor_h20+rx) & mask;
                y_h = x_h^w_h[20];
                uint32_t g20 = (y_h-e_xor_d20-ry) & mask;
                down[5] = g20; 
                            
                clockarrow(20,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
                /*t=21*/
                             
                uint32_t a21 = up[17];
                uint32_t b21 = up[16];
                uint32_t c21 = up[7];
                uint32_t d21 = up[6];
                uint32_t e21 = down[13];
                uint32_t f21 = down[12];                           
                uint32_t g21 = down[5];
                uint32_t h21 = down[4]; 
                            
                x_l = (b21+(e21^d21)) & mask;
                y_l = (f21+(a21^h21)) & mask;
                                                                        
                rx = (b21+(e21^d21)) >> s;
                ry = (f21+(a21^h21)) >> s;
                x_h = (c21+(a21^h21)+rx) & mask;
                y_h = (g21+(e21^d21)+ry) & mask ;  
                                                     
                clockarrow(21,up,down,w_l,w_h,x_l,x_h,y_l,y_h);                         
                                
                uint32_t a22 = up[17];
                uint32_t b22 = up[16];
                uint32_t c22 = up[7];
                uint32_t d22 = up[6];
                uint32_t e22 = down[13];
                uint32_t f22 = down[12];
                            
                uint32_t e_xor_d22 = e22^d22;
                x_l = (b22+e_xor_d22) & mask;
                rx = (b22+e_xor_d22) >> s;
                y_l = x_l^w_l[22];
                uint32_t a_xor_h22 = (y_l - f22) & mask;
                ry = (f22+a_xor_h22) >> s;
                uint32_t h22 = a_xor_h22^a22;
                down[4] = h22;
                x_h = (c22+a_xor_h22+rx) & mask;
                y_h = x_h^w_h[22];
                uint32_t g22 = (y_h-e_xor_d22-ry) & mask;
                down[5] = g22;

                                
                clockarrow(22,up,down,w_l,w_h,x_l,x_h,y_l,y_h);
                if(check(s,up,down,23,w_l,w_h,5,mask)){
                    printf("found one \n");
                    res ++;               
                    
                }
                
            }
        }
          
        }/* end for n3*/
        }/* end for n2*/
        }/* end for n1*/
        } /*end for n0*/
        
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time %f \n",cpu_time_used);        
    return res;
    }/*end fonction*/
    

   
int main(){
    uint32_t x[9];
    uint32_t y[7];
    uint32_t w[40];
    uint32_t s;

    uint32_t res;
    
    s=4;
    initialisation(x,y,w,s);
    res = attack(w,s);
    printf("%i \n",res);    

    return 0;
    }






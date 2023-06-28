#include <stdlib.h>
#include <stdio.h> /* function printf*/
#include <math.h>
#include <stdint.h>
#include <string.h> /* mcpy*/
#include <time.h>

/* Warning! this code works well with s=4 (N=8) but not for s=16 (N=32) (uint32_t is too small). In the case N=32, the code would not run on a practical computeur.*/



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

void initialisation(uint32_t* x, uint32_t* y, uint32_t* w, uint32_t s, uint32_t m){
/*step 1: create the internal states
(warning ! because of rand the internal states are always the same) */
    uint32_t i;
    uint32_t j;
    uint32_t temp_x;
    uint32_t temp_y;
    uint32_t MASK;
    
    MASK = expo(2,2*s)-1;

    printf("x_0 = [");
    for (i=0;i<31;i++){
        x[i] = rand() & MASK;
        printf("%i, ",x[i]);
        }   
    printf("]\n");

    printf("y_0 = [");
    for (i=0;i<17;i++){
        y[i] = rand() & MASK;
        printf("%i, ",y[i]);
        }
    printf("]\n");
/*step 2: run the PRNG
(warning! we erased the original internal states) */
    for(i=0;i<m;i++){
        temp_x = ( (x[30]^(y[2]<<s)) + (x[2]^(y[16]>>s)) ) & MASK;
        temp_y = ( (y[16]^(x[2]<<s)) + (y[2]^(x[30]>>s)) ) & MASK;
        w[i] = temp_x^temp_y;
        
        for (j=30;j>0;j--){
            x[j] = x[j-1];
            }
        x[0] = temp_x;
        
        for (j=16;j>0;j--){
            y[j] = y[j-1];
        }
        y[0] = temp_y;
    }

    printf("x_243 = [");
    for (i=0;i<31;i++){
        printf("%i, ",x[i]);
        }   
    printf("]\n");

    printf("y_243 = [");
    for (i=0;i<17;i++){
        printf("%i, ",y[i]);
        }
    printf("]\n");
    return;
}

void copy(uint32_t* dep, uint32_t* arr, int size){
    memcpy((void*) arr, (void*) dep, size*sizeof(uint32_t)); 
}  
    
    
void one_clock(uint32_t t, uint32_t* up, uint32_t* down, uint32_t* w_l, uint32_t* w_h, uint32_t* res){
/*we modify the list, so we have to deep copy if needed*/   
    uint32_t temp[60];
    uint32_t x_l = res[0];
    uint32_t y_l = res[1];
    uint32_t x_h = res[2];
    uint32_t y_h = res[3];
    
    /*Mirror*/
    up[32] = down[32] ^ w_l[t-17];
    up[33] = down[33] ^ w_h[t-17];
    /*shift*/
    
    copy(up,temp,60);
    copy(temp,up+2,60);
    /*for(i=17;i>1;i--) up[i] = up[i-2];*/
    up[1] = x_h;
    up[0] = x_l;
    
    /*for(i=13;i>1;i--) down[i] = down[i-2];*/
    copy(down,temp,32);
    copy(temp,down+2,32);
    down[1] = y_h;
    down[0] = y_l;
    return;
}


    
void clocks(uint32_t t, uint32_t* up, uint32_t* down, uint32_t* w_l, uint32_t* w_h, uint32_t* res, uint32_t nb_clock){
    /* we clock the generator*/
    uint32_t i=0;
    one_clock(t,up,down,w_l,w_h,res);
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;
    for(i=1;i<nb_clock;i++) one_clock(t+i,up,down,w_l,w_h,res);
}

void save(uint32_t* up, uint32_t* down, uint32_t* up0, uint32_t* down0){
    copy(up, up0, 62);
    copy(down, down0, 34);
    }
    
int situation_a(uint32_t s,uint32_t t, uint32_t* w_l, uint32_t* w_h, uint32_t* up,uint32_t* down, uint32_t a, uint32_t* res){
    uint32_t mask = expo(2,s)-1;
    /*we guess a*/
    uint32_t c = up[5];
    uint32_t d = up[4];
    uint32_t e = down[33];          
    uint32_t f = down[32];           
    uint32_t g = down[5];
    uint32_t h = down[4];
                    
    uint32_t a_xor_h = a^h;
    uint32_t e_xor_d = e^d;
                
    uint32_t y_l = (f+a_xor_h) & mask;
    uint32_t ry = (f+a_xor_h) >> s;
    uint32_t x_l = w_l[t]^y_l;
    uint32_t b = (x_l - e_xor_d) & mask;
    uint32_t rx = (b+e_xor_d) >> s;
    uint32_t y_h = (g+e_xor_d+ry) & mask;
    uint32_t x_h = (a_xor_h + c +rx) & mask;
    
    res[0] = x_l;
    res[1] = y_l;
    res[2] = x_h;
    res[3] = y_h;
    
    if ((x_h^y_h) == w_h[t]){
        return 1;
    }
    
    else{
        return 0;
    }

}

int situation_gh(uint32_t s,uint32_t t, uint32_t* w_l,uint32_t* w_h, uint32_t* up,uint32_t* down, uint32_t g, uint32_t h, uint32_t* res){
    /* we guess g and h*/
    uint32_t mask = expo(2,s)-1;
    uint32_t c = g^w_h[t-3];
    uint32_t d = h^w_l[t-3];
            
    uint32_t a = up[61];
    uint32_t e = down[33];          
    uint32_t f = down[32];          
            
    up[5] = c;
    up[4] = d;
    down[5] = g;
    down[4] = h;
            
    uint32_t a_xor_h = a^h;
    uint32_t e_xor_d = e^d;
            
    uint32_t y_l = (f+a_xor_h) & mask;
    uint32_t ry = (f+a_xor_h) >> s;
    uint32_t x_l = w_l[t]^y_l;
    uint32_t b = (x_l - e_xor_d) & mask;
    uint32_t rx = (b+e_xor_d) >> s;
    uint32_t y_h = (g+e_xor_d+ry) & mask;
    uint32_t x_h = (a_xor_h + c +rx) & mask;
                
    res[0] = x_l;
    res[1] = y_l;
    res[2] = x_h;
    res[3] = y_h;

    if ((x_h^y_h) == w_h[t]){
        return 1;
    }
    
    else{
        return 0;
    }    
}
    
int situation_zero(uint32_t s,uint32_t t, uint32_t* w_l, uint32_t* w_h, uint32_t* up,uint32_t* down, uint32_t* res){
    /* we guess nothing*/
    uint32_t mask = expo(2,s)-1;
    uint32_t a = up[61];
    uint32_t b = up[60];
    uint32_t c = up[5];
    uint32_t d = up[4];
    uint32_t e = down[33] ;          
    uint32_t f = down[32];
    uint32_t g = down[5];
    uint32_t h = down[4];
                                    
    uint32_t a_xor_h = a^h;
    uint32_t e_xor_d = e^d;
                                
    uint32_t x_l = (b + e_xor_d) & mask;                
    uint32_t rx = (b + e_xor_d) >> s;
    uint32_t y_l = (f+a_xor_h) & mask;
    uint32_t ry = (f+a_xor_h) >> s    ;                
    uint32_t x_h= (a_xor_h+c+rx) & mask;
    uint32_t y_h = (g+e_xor_d+ry) & mask;  
    
    res[0] = x_l;
    res[1] = y_l;
    res[2] = x_h;
    res[3] = y_h;

    if(((x_l^y_l) == w_l[t]) && ((x_h^y_h)==w_h[t])){
        return 1;
    }
    
    else{
        return 0;
    } 
}

int suite_zero(uint32_t s,uint32_t t, uint32_t* w_l, uint32_t* w_h, uint32_t* up,uint32_t* down, uint32_t* res, uint32_t param){
    /* we apply situtaion_zero several times in a row*/
    uint32_t i=0;
    while(i<param){
        if(situation_zero(s,t+i,w_l,w_h,up,down,res)){
            clocks(t+i,up,down,w_l,w_h,res,1);
            i++ ;
        }
        else{
            return 0;
        }
    }
    return 1;
}
    
void attack_arrow(uint32_t* w,uint32_t s, uint32_t m){
    uint32_t compteur[100];
    uint32_t w_h[m];
    uint32_t w_l[m];
    
    uint32_t mask = expo(2,s)-1;
    uint32_t i;
    
    for(i=0;i<100;i++){
        compteur[i] = 0;
    }
    
    for(i=0;i<m;i++){
        w_l[i] = w[i] & mask;
        w_h[i] = w[i] >> s;
    }
    
    uint32_t n0;
    for(n0=0;n0<expo(2,5*s);n0 ++){
        uint32_t up[64];
        uint32_t down[34];
        uint32_t res[4];       
        /*t=17*/
        
        uint32_t a17 = n0 & mask;
        uint32_t e17 = (n0 >> s) & mask;
        uint32_t f17 = (n0 >> (2*s)) & mask;
        uint32_t g17 = (n0 >> (3*s)) & mask;
        uint32_t h17 = (n0 >> (4*s)) & mask;
        
        
        uint32_t c17 = g17^w_h[17-3];
        uint32_t d17 = h17^w_l[17-3];
        
        up[61] = a17;
        down[33] = e17;
        down[32] = f17;
        up[5] = c17;
        up[4] = d17;
        down[5] = g17;
        down[4] = h17;

        uint32_t a_xor_h17 = a17^h17;
        uint32_t e_xor_d17 = e17^d17;
        
        uint32_t y_l = (f17+a_xor_h17) & mask;
        uint32_t ry = (f17+a_xor_h17) >> s;
        uint32_t x_l = w_l[17]^y_l;
        uint32_t b = (x_l - e_xor_d17) & mask;
        uint32_t rx = (b+e_xor_d17) >> s;
        uint32_t y_h = (g17+e_xor_d17+ry) & mask;
        uint32_t x_h = (a_xor_h17 + c17 +rx) & mask;
        
        res[0] = x_l;
        res[1] = y_l;
        res[2] = x_h;
        res[3] = y_h;
        
        if((x_h^y_h) == w_h[17]){
            clocks(17,up,down,w_l,w_h,res,14);
            uint32_t up_t31[62];
            uint32_t down_t31[34];
            save(up,down,up_t31,down_t31);

        uint32_t n1;
        for(n1=0;n1<expo(2,2*s);n1++){
            save(up_t31,down_t31,up,down);
            uint32_t g31 = n1 & mask;
            uint32_t h31 = (n1 >> s) & mask;

            if (situation_gh(s,31,w_l,w_h,up,down,g31,h31,res)){
                compteur[0] +=1;      
                clocks(31,up,down,w_l,w_h,res,3);
                uint32_t up_t34[62];
                uint32_t down_t34[34];
                save(up,down,up_t34,down_t34);    
    
        uint32_t n2;
        for(n2=0;n2<expo(2,s);n2++){
            save(up_t34,down_t34,up,down);
            uint32_t a34 = n2;
            up[61] = a34;
            if (situation_a(s,34,w_l,w_h,up,down,a34,res)){
                compteur[1] +=1;
                clocks(34,up,down,w_l,w_h,res,11);
                uint32_t up_t45[62];
                uint32_t down_t45[34];                
                save(up,down,up_t45,down_t45);

        uint32_t n3;
        for(n3=0;n3<expo(2,2*s);n3++){
            save(up_t45,down_t45,up,down);
            uint32_t g45 = n3 & mask;
            uint32_t h45 = (n3 >> s) & mask; 
            if(situation_gh(s,45,w_l,w_h,up,down,g45,h45,res)){
                compteur[2] +=1;
                clocks(45,up,down,w_l,w_h,res,3);
                if(situation_zero(s,48,w_l,w_h,up,down,res)){
                    compteur[3] +=1;
                    clocks(48,up,down,w_l,w_h,res,3);
                    uint32_t up_t51[62];
                    uint32_t down_t51[34];                     
                    save(up,down,up_t51,down_t51);
                    
        uint32_t n4;                                                                  
        for(n4=0; n4< expo(2,s);n4++){
            uint32_t a51 = n4;
            save(up_t51,down_t51,up,down);
            up[61] = a51;
            if(situation_a(s,51,w_l,w_h,up,down,a51,res)){
                compteur[4] +=1 ;
                clocks(51,up,down,w_l,w_h,res,8);            
                uint32_t up_t59[62];
                uint32_t down_t59[34];                
                save(up,down,up_t59,down_t59);       

        uint32_t n5;
        for(n5=0;n5<expo(2,2*s);n5++){
            save(up_t59,down_t59,up,down);
            uint32_t g59 = n5 & mask;
            uint32_t h59 = (n5 >> s) & mask;
            if(situation_gh(s,59,w_l,w_h,up,down,g59,h59,res)){
                compteur[5] +=1;
                clocks(59,up,down,w_l,w_h,res,3);
                if(situation_zero(s,62,w_l,w_h,up,down,res)){
                    compteur[6] +=1;   
                    clocks(62,up,down,w_l,w_h,res,3);
                    if(situation_zero(s,65,w_l,w_h,up,down,res)){
                        compteur[7] +=1;
                        clocks(65,up,down,w_l,w_h,res,3);
                        uint32_t up_t68[62];
                        uint32_t down_t68[34];                
                        save(up,down,up_t68,down_t68);                         

        uint32_t n6;
        for(n6=0; n6<expo(2,s); n6++){
            uint32_t a68=n6;
            save(up_t68,down_t68,up,down);
            up[61] = a68;
            if(situation_a(s,68,w_l,w_h,up,down,a68,res)){
                compteur[8] +=1;
                clocks(68,up,down,w_l,w_h,res,5);
                uint32_t up_t73[62];
                uint32_t down_t73[34];                
                save(up,down,up_t73,down_t73);
        
        uint32_t n7;                                                                
        for(n7=0;n7<expo(2,2*s);n7++){
            save(up_t73,down_t73,up,down);
            uint32_t g73 = n7 & mask;
            uint32_t h73 = (n7 >> s) & mask;
            if(situation_gh(s,73,w_l,w_h,up,down,g73,h73,res)){
                compteur[9] +=1;
                clocks(73,up,down,w_l,w_h,res,3);
                if(situation_zero(s,76,w_l,w_h,up,down,res)){
                    compteur[10] +=1;
                    clocks(76,up,down,w_l,w_h,res,3);
                    if(situation_zero(s,79,w_l,w_h,up,down,res)){
                        compteur[11] +=1;
                        clocks(79,up,down,w_l,w_h,res,3);
                        if(situation_zero(s,82,w_l,w_h,up,down,res)){
                            compteur[12] +=1;
                            clocks(82,up,down,w_l,w_h,res,3);
                            uint32_t up_t85[62];
                            uint32_t down_t85[34];                
                            save(up,down,up_t85,down_t85);
        
        uint32_t n8;                           
        for(n8=0;n8<expo(2,s);n8++){
            uint32_t a85=n8;
            save(up_t85,down_t85,up,down);
            up[61] = a85;
            if(situation_a(s,85,w_l,w_h,up,down,a85,res)){
                compteur[13] +=1;
                clocks(85,up,down,w_l,w_h,res,2);
                uint32_t up_t87[62];
                uint32_t down_t87[34];                
                save(up,down,up_t87,down_t87);
        
        uint32_t n9;
        for(n9=0;n9<expo(2,2*s);n9++){ 
            save(up_t87,down_t87,up,down);
            uint32_t g87 = n9 & mask;
            uint32_t h87 = (n9 >> s) & mask;
            if(situation_gh(s,87,w_l,w_h,up,down,g87,h87,res)){
                compteur[14] +=1;
                clocks(87,up,down,w_l,w_h,res,3);
                if(situation_zero(s,90,w_l,w_h,up,down,res)){
                    compteur[15] +=1;
                    clocks(90,up,down,w_l,w_h,res,3);
                    if(situation_zero(s,93,w_l,w_h,up,down,res)){
                        compteur[16] +=1;
                        clocks(93,up,down,w_l,w_h,res,3);
                        if(situation_zero(s,96,w_l,w_h,up,down,res)){
                            compteur[17] +=1;
                            clocks(96,up,down,w_l,w_h,res,3);
                            if(situation_zero(s,99,w_l,w_h,up,down,res)){
                                compteur[18] +=1;
                                clocks(99,up,down,w_l,w_h,res,2);
                                uint32_t up_t101[62];
                                uint32_t down_t101[34];                
                                save(up,down,up_t101,down_t101);
        
        uint32_t n10; 
        for(n10=0;n10<expo(2,2*s);n10++){
            save(up_t101,down_t101, up, down);
            uint32_t g101 = n10 & mask;
            uint32_t h101 = (n10 >> s) & mask;
            if(situation_gh(s,101,w_l,w_h,up,down,g101,h101,res)){
                compteur[19] +=1;
                clocks(101,up,down,w_l,w_h,res,1);
                uint32_t up_t102[62];
                uint32_t down_t102[34];                
                save(up,down,up_t102,down_t102);                
                
        uint32_t n11;
        for(n11=0;n11<expo(2,s);n11++){
            uint32_t a102=n11;
            save(up_t102,down_t102,up,down);
            up[61] = a102;
            if(situation_a(s,102,w_l,w_h,up,down,a102,res)){
                compteur[20] +=1;
                clocks(102,up,down,w_l,w_h,res,2);
                uint32_t up_t104[62];
                uint32_t down_t104[34];                
                save(up,down,up_t104,down_t104);

        uint32_t n12;                                                   
        for(n12=0;n12<expo(2,2*s);n12++){
            save(up_t104,down_t104, up, down);
            uint32_t g104 = n12 & mask;
            uint32_t h104 = (n12 >> s) & mask;
            if(situation_gh(s,104,w_l,w_h,up,down,g104,h104,res)){
                compteur[21] +=1;
                clocks(104,up,down,w_l,w_h,res,3);
                if(situation_zero(s,107,w_l,w_h,up,down,res)){
                    compteur[22] +=1;
                    clocks(107,up,down,w_l,w_h,res,3);
                    if(situation_zero(s,110,w_l,w_h,up,down,res)){
                        compteur[23] +=1;
                        clocks(110,up,down,w_l,w_h,res,3);
                        if(situation_zero(s,113,w_l,w_h,up,down,res)){
                            compteur[24] +=1;
                            clocks(113,up,down,w_l,w_h,res,2);
                            uint32_t up_t115[62];
                            uint32_t down_t115[34];
                            save(up,down,up_t115,down_t115);
                            
        uint32_t n13;             
        for(n13=0;n13<expo(2,2*s);n13++){
            save(up_t115,down_t115, up, down);
            uint32_t g115 = n13 & mask;
            uint32_t h115 = (n13 >> s) & mask;
            if(situation_gh(s,115,w_l,w_h,up,down,g115,h115,res)){
                compteur[25] +=1;
                clocks(115,up,down,w_l,w_h,res,1);
                if(situation_zero(s,116,w_l,w_h,up,down,res)){
                    compteur[26] +=1;
                    clocks(116,up,down,w_l,w_h,res,2);
                    if(situation_zero(s,118,w_l,w_h,up,down,res)){
                        compteur[27] +=1;
                        clocks(118,up,down,w_l,w_h,res,1);                
                        uint32_t up_t119[62];
                        uint32_t down_t119[34];                
                        save(up,down,up_t119,down_t119);
                
        uint32_t n14;                                 
        for(n14=0;n14<expo(2,s);n14++){
            uint32_t a119=n14;
            save(up_t119,down_t119,up,down);
            up[61] = a119;
            if(situation_a(s,119,w_l,w_h,up,down,a119,res)){    
                compteur[28] +=1;   
                clocks(119,up,down,w_l,w_h,res,2);
                uint32_t up_t121[62];
                uint32_t down_t121[34];                
                save(up,down,up_t121,down_t121);
         
        uint32_t n15;
        for(n15 =0;n15<expo(2,2*s);n15++){
            save(up_t121,down_t121, up, down);
            uint32_t g121 = n15 & mask;
            uint32_t h121 = (n15 >> s) & mask;
            if(situation_gh(s,121,w_l,w_h,up,down,g121,h121,res)){                
                compteur[29] +=1;
                clocks(121,up,down,w_l,w_h,res,3);
                if(situation_zero(s,124,w_l,w_h,up,down,res)){
                    compteur[30] +=1;
                    clocks(124,up,down,w_l,w_h,res,3);
                    if(situation_zero(s,127,w_l,w_h,up,down,res)){
                        compteur[31] +=1;
                        clocks(127,up,down,w_l,w_h,res,2);         
                        uint32_t up_t129[62];
                        uint32_t down_t129[34];                
                        save(up,down,up_t129,down_t129);
                        
        uint32_t n16;
        for(n16=0;n16<expo(2,2*s);n16++){
            save(up_t129,down_t129, up, down);
            uint32_t g129 = n16 & mask;
            uint32_t h129 = (n16 >> s) & mask;
            if(situation_gh(s,129,w_l,w_h,up,down,g129,h129,res)){
                compteur[32] +=1;
                clocks(129,up,down,w_l,w_h,res,1);
                if(situation_zero(s,130,w_l,w_h,up,down,res)){
                    compteur[33] +=1;
                    clocks(130,up,down,w_l,w_h,res,2);
                    if(situation_zero(s,132,w_l,w_h,up,down,res)){                
                        compteur[34] +=1;
                        clocks(132,up,down,w_l,w_h,res,1);         
                        if(situation_zero(s,133,w_l,w_h,up,down,res)){
                            compteur[35] +=1;
                            clocks(133,up,down,w_l,w_h,res,2);
                            if(situation_zero(s,135,w_l, w_h,up,down,res)){
                                compteur[36] +=1;
                                clocks(135,up,down,w_l,w_h,res,1);
                                uint32_t up_t136[62];
                                uint32_t down_t136[34];                
                                save(up,down,up_t136,down_t136);

        uint32_t n17;
        for(n17=0; n17<expo(2,s);n17++){
            save(up_t136,down_t136, up, down);
            uint32_t a136 = n17;
            if(situation_a(s,136,w_l,w_h,up,down,a136,res)){
                compteur[37] +=1;
                clocks(136,up,down,w_l,w_h,res,2);
                if(situation_zero(s,138,w_l,w_h,up,down,res)){                
                    compteur[38] +=1;
                    clocks(138,up,down,w_l,w_h,res,3);         
                    if(situation_zero(s,141,w_l,w_h,up,down,res)){
                        compteur[39] +=1;
                        clocks(141,up,down,w_l,w_h,res,2);
                        uint32_t up_t143[62];
                        uint32_t down_t143[34];                
                        save(up,down,up_t143,down_t143);                    
                        
        uint32_t n18;
        for(n18=0; n18<expo(2,2*s); n18++){
            save(up_t143,down_t143, up, down);
            uint32_t g143 = n18 & mask;
            uint32_t h143 = (n18 >> s) & mask;
            if(situation_gh(s,143,w_l,w_h,up,down,g143,h143,res)){
                compteur[40] +=1;
                clocks(143,up,down,w_l,w_h,res,1);
                if(situation_zero(s,144,w_l,w_h,up,down,res)){
                    compteur[41] +=1;
                    clocks(144,up,down,w_l,w_h,res,2);
                    if(situation_zero(s,146,w_l,w_h,up,down,res)){                
                        compteur[42] +=1;
                        clocks(146,up,down,w_l,w_h,res,1);         
                        if(situation_zero(s,147,w_l,w_h,up,down,res)){
                            compteur[43] +=1;
                            clocks(147,up,down,w_l,w_h,res,2);
                            if(situation_zero(s,149,w_l,w_h,up,down,res)){
                                compteur[44] +=1;
                                clocks(149,up,down,w_l,w_h,res,1);
                                if(situation_zero(s,150,w_l,w_h,up,down,res)){
                                    compteur[45] +=1;
                                    clocks(150,up,down,w_l,w_h,res,2);
                                    if(situation_zero(s,152,w_l,w_h,up,down,res)){
                                        compteur[46] +=1;
                                        clocks(152,up,down,w_l,w_h,res,1);
                                        uint32_t up_t153[62];
                                        uint32_t down_t153[34];                
                                        save(up,down,up_t153,down_t153);
  
  
        uint32_t n19;                              
        for(n19=0;n19<expo(2,s);n19++){
            uint32_t a153=n19;
            save(up_t153,down_t153,up,down);
            up[61] = a153;
            if(situation_a(s,153,w_l,w_h,up,down,a153,res)){    
                compteur[47] +=1;   
                clocks(153,up,down,w_l,w_h,res,2);
                if(situation_zero(s,155,w_l,w_h,up,down,res)){
                    compteur[48] += 1;
                    clocks(155,up,down,w_l,w_h,res,2);
                    uint32_t up_t157[62];
                    uint32_t down_t157[34];                
                    save(up,down,up_t157,down_t157);
                

        uint32_t n20;
        for(n20=0; n20<expo(2,2*s); n20++){
            save(up_t157,down_t157, up, down);
            uint32_t g157 = n20 & mask;
            uint32_t h157 = (n20 >> s) & mask;
            if(situation_gh(s,157,w_l,w_h,up,down,g157,h157,res)){
                compteur[49] +=1;
                clocks(157,up,down,w_l,w_h,res,1);
                if(situation_zero(s,158,w_l,w_h,up,down,res)){
                    compteur[50] +=1;
                    clocks(158,up,down,w_l,w_h,res,2);
                    if(situation_zero(s,160,w_l,w_h,up,down,res)){                
                        compteur[51] +=1;
                        clocks(160,up,down,w_l,w_h,res,1);         
                        if(situation_zero(s,161,w_l,w_h,up,down,res)){
                            compteur[52] +=1;
                            clocks(161,up,down,w_l,w_h,res,2);
                            if(situation_zero(s,163,w_l,w_h,up,down,res)){
                                compteur[53] +=1;
                                clocks(163,up,down,w_l,w_h,res,1);
                                if(situation_zero(s,164,w_l,w_h,up,down,res)){
                                    compteur[54] +=1;
                                    clocks(164,up,down,w_l,w_h,res,2);
                                    if(situation_zero(s,166,w_l,w_h,up,down,res)){
                                        compteur[55] +=1;
                                        clocks(166,up,down,w_l,w_h,res,1);
                                        if(situation_zero(s,167,w_l,w_h,up,down,res)){
                                            compteur[56] +=1;
                                            clocks(167,up,down,w_l,w_h,res,2);
                                            if(situation_zero(s,169,w_l,w_h,up,down,res)){
                                                compteur[57] +=1;
                                                clocks(169,up,down,w_l,w_h,res,1);
                                                uint32_t up_t170[62];
                                                uint32_t down_t170[34];                
                                                save(up,down,up_t170,down_t170);        
                
                
        uint32_t n21;
        for(n21=0;n21<expo(2,s);n21++){
            uint32_t a170=n21;
            save(up_t170,down_t170,up,down);
            up[61] = a170;
            if(situation_a(s,170,w_l,w_h,up,down,a170,res)){    
                compteur[58] +=1;   
                clocks(170,up,down,w_l,w_h,res,1);
                uint32_t up_t171[62];
                uint32_t down_t171[34];                
                save(up,down,up_t171,down_t171);
                
        uint32_t n22;
        for(n22=0; n22<expo(2,2*s); n22++){
            save(up_t171,down_t171, up, down);
            uint32_t g171 = n22 & mask;
            uint32_t h171 = (n22 >> s) & mask;
            if(situation_gh(s,171,w_l,w_h,up,down,g171,h171,res)){
                compteur[59] +=1;
                clocks(171,up,down,w_l,w_h,res,1);
                if(situation_zero(s,172,w_l,w_h,up,down,res)){
                    compteur[60] +=1;
                    clocks(172,up,down,w_l,w_h,res,2);
                    if(suite_zero(s,174,w_l,w_h,up,down,res,2)){                
                        compteur[61] +=1;
                        clocks(176,up,down,w_l,w_h,res,1);         
                        if(suite_zero(s,177,w_l,w_h,up,down,res,2)){
                            compteur[62] +=1;
                            clocks(179,up,down,w_l,w_h,res,1);
                            if(suite_zero(s,180,w_l,w_h,up,down,res,2)){
                                compteur[63] +=1;
                                clocks(182,up,down,w_l,w_h,res,1);
                                if(suite_zero(s,183,w_l,w_h,up,down,res,2)){
                                    compteur[64] +=1;
                                    uint32_t up_t185[62];
                                    uint32_t down_t185[34]; 
                                    save(up,down,up_t185,down_t185);
                                    

        uint32_t n23;
        for(n23=0; n23<expo(2,2*s); n23++){
            save(up_t185,down_t185, up, down);
            uint32_t g185 = n23 & mask;
            uint32_t h185 = (n23 >> s) & mask;
            if(situation_gh(s,185,w_l,w_h,up,down,g185,h185,res)){
                compteur[65] +=1;
                clocks(185,up,down,w_l,w_h,res,1);
                if(situation_zero(s,186,w_l,w_h,up,down,res)){
                    compteur[66] +=1;
                    clocks(186,up,down,w_l,w_h,res,1);
                    uint32_t up_t187[62];
                    uint32_t down_t187[34]; 
                    save(up,down,up_t187,down_t187);
                                                    
        uint32_t n24;
        for(n24=0;n24<expo(2,s);n24++){
            uint32_t a187=n24;
            save(up_t187,down_t187,up,down);
            up[61] = a187;
            if(situation_a(s,187,w_l,w_h,up,down,a187,res)){    
                compteur[67] +=1;   
                clocks(187,up,down,w_l,w_h,res,1);
                if(suite_zero(s,188,w_l,w_h,up,down,res,2)){
                    compteur[68] +=1;
                    clocks(190,up,down,w_l,w_h,res,1);
                    if(suite_zero(s,191,w_l,w_h,up,down,res,2)){
                        compteur[69] +=1;
                        clocks(193,up,down,w_l,w_h,res,1);
                            if(suite_zero(s,194,w_l,w_h,up,down,res,2)){
                                compteur[70] +=1;
                                clocks(196,up,down,w_l,w_h,res,1);
                                if(suite_zero(s,197,w_l,w_h,up,down,res,2)){
                                    compteur[71] +=1;
                                    uint32_t up_t199[62];
                                    uint32_t down_t199[34]; 
                                    save(up,down,up_t199,down_t199);
                                                                                       
        
        uint32_t n25;
        for(n25=0; n25<expo(2,2*s); n25++){
            save(up_t199,down_t199, up, down);
            uint32_t g199 = n25 & mask;
            uint32_t h199 = (n25 >> s) & mask;
            if(situation_gh(s,199,w_l,w_h,up,down,g199,h199,res)){
                compteur[72] +=1;
                clocks(199,up,down,w_l,w_h,res,1);
                if(suite_zero(s,200,w_l,w_h,up,down,res,4)){
                    compteur[73] +=1;
                    uint32_t up_t204[62];
                    uint32_t down_t204[34]; 
                    save(up,down,up_t204,down_t204);

        uint32_t n26;
        for(n26=0;n26<expo(2,s);n26++){
            uint32_t a204=n26;
            save(up_t204,down_t204,up,down);
            up[61] = a204;
            if(situation_a(s,204,w_l,w_h,up,down,a204,res)){    
                compteur[74] +=1;   
                clocks(204,up,down,w_l,w_h,res,1);
                if(suite_zero(s,205,w_l,w_h,up,down,res,2)){
                    compteur[75] +=1;
                    clocks(207,up,down,w_l,w_h,res,1);
                    if(suite_zero(s,208,w_l,w_h,up,down,res,2)){
                        compteur[76] +=1;
                        clocks(210,up,down,w_l,w_h,res,1);
                            if(suite_zero(s,211,w_l,w_h,up,down,res,2)){
                                compteur[77] +=1;
                                uint32_t up_t213[62];
                                uint32_t down_t213[34]; 
                                save(up,down,up_t213,down_t213);
                                                                             
        uint32_t n27;
        for(n27=0; n27<expo(2,2*s); n27++){
            save(up_t213,down_t213, up, down);
            uint32_t g213 = n27 & mask;
            uint32_t h213 = (n27 >> s) & mask;
            if(situation_gh(s,213,w_l,w_h,up,down,g213,h213,res)){
                compteur[77] +=1;
                clocks(213,up,down,w_l,w_h,res,1);
                if(suite_zero(s,214,w_l,w_h,up,down,res,7)){
                    compteur[78] +=1;
                    uint32_t up_t221[62];
                    uint32_t down_t221[34]; 
                    save(up,down,up_t221,down_t221);

        uint32_t n28;
        for(n28=0;n28<expo(2,s);n28++){
            uint32_t a221=n28;
            save(up_t221,down_t221,up,down);
            up[61] = a221;
            if(situation_a(s,221,w_l,w_h,up,down,a221,res)){    
                compteur[79] +=1;   
                clocks(221,up,down,w_l,w_h,res,1);
                if(suite_zero(s,222,w_l,w_h,up,down,res,2)){
                    compteur[80] +=1;
                    clocks(224,up,down,w_l,w_h,res,1);
                    if(suite_zero(s,225,w_l,w_h,up,down,res,2)){
                        compteur[81] +=1;
                        uint32_t up_t227[62];
                        uint32_t down_t227[34]; 
                        save(up,down,up_t227,down_t227);

        uint32_t n29;
        for(n29=0; n29<expo(2,2*s); n29++){
            save(up_t227,down_t227, up, down);
            uint32_t g227 = n29 & mask;
            uint32_t h227 = (n29 >> s) & mask;
            if(situation_gh(s,227,w_l,w_h,up,down,g227,h227,res)){
                compteur[82] +=1;
                clocks(227,up,down,w_l,w_h,res,1);
                if(suite_zero(s,228,w_l,w_h,up,down,res,10)){
                    compteur[83] +=1;
                    uint32_t up_t238[62];
                    uint32_t down_t238[34]; 
                    save(up,down,up_t238,down_t238);
                    
        uint32_t n30;
        for(n30=0;n30<expo(2,s);n30++){
            uint32_t a238=n30;
            save(up_t238,down_t238,up,down);
            up[61] = a238;
            if(situation_a(s,238,w_l,w_h,up,down,a238,res)){    /* here the internal states are all guessed */
                compteur[84] +=1;   
                clocks(238,up,down,w_l,w_h,res,1);
                if(suite_zero(s,239,w_l,w_h,up,down,res,4)){   /* here we check the guesses */
                    compteur[85] +=1;           
                        printf("found x_243 = [");
                        for (i=0;i<31;i++){
                            printf("%i, ",up[2*i]+up[2*i + 1]*expo(2,s));
                            }   
                        printf("]\n");

                        printf("found y_243 = [");
                        for (i=0;i<17;i++){
                            printf("%i, ",down[2*i]+down[2*i + 1]*expo(2,s));
                        }
                        printf("]\n");
                    

                               
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
    return ;
}

int main(){
    uint32_t x[31];
    uint32_t y[17];
    uint32_t w[250];

    uint32_t s;
    uint32_t m;

    srand(time(NULL));
    
    s=5;
    m=243;
    initialisation(x,y,w,s,m);
    clock_t start ;
    clock_t stop ;
    double total ;

    start = clock () ; 
    attack_arrow(w,s,m);
    stop = clock () ; 
    total  = (( double ) ( stop - start ) ) / CLOCKS_PER_SEC ;
    printf ( "%f ", time) ;

    
    return 0;
    }

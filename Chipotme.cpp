
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <string> 
#include <fstream>
#include <iostream>
#include<cmath>

using namespace std;

// Global file for streams:

fstream newfile;
string textfilename;
bool end_of_stream=false; 
double clk=0;
double prclk=0;

// items are strings, item-sets are vectors of strings 
// Global variables: 



// Qr and Qc is a queue and they hold unordered maps corresponding to counts of itemsets of each transaction in slices of windows corresponding to the reference and current window 
// Br and Bc hold count of frequency of singletons respectively in the reference and current windows  
// Hr and Hc hold the frequent singletons in the reference and current windows 
// C   itemsets which change 



// all of the global data structures are unordered maps which is rb tree
std::unordered_map<string, int>  Br ={};
std::unordered_map<string, int>  Bc ={};
std::queue<std::unordered_map<string, int>>  Qr ;
std::queue<std::unordered_map<string, int>>  Qc;
std::unordered_map<string, int>  Hr ={};
std::unordered_map<string, int>  Hc ={};
std::unordered_map<string, int>  C ={};


// algorithm paramethers 


double eps;
double eps_inv;
int max_transaction_size;
// scope takes values: 2,3,4,5
int scope; 
int delta_inv;
int samples;


int queue_size;
int b;
int window_size;


double g;
double thresh;


// queues for count of 2-itemsets 
std::queue<std::unordered_map<string, int>>  Q2r ;
std::queue<std::unordered_map<string, int>>  Q2c;


// queues for count of 3-itemsets 
std::queue<std::unordered_map<string, int>>  Q3r ;
std::queue<std::unordered_map<string, int>>  Q3c;

// queues for count of 4-itemsets 
std::queue<std::unordered_map<string, int>>  Q4r ;
std::queue<std::unordered_map<string, int>>  Q4c;

// queues for count of 5-itemsets 
std::queue<std::unordered_map<string, int>>  Q5r ;
std::queue<std::unordered_map<string, int>>  Q5c;





// printing functions 
void Print_vec(std::vector<string> vec, string name){
    std::cout << "started printing map  "<< name << std::endl; 
    for (auto i : vec){
         std::cout << i << ",";

    }
    std::cout << std::endl;
}
void Print_map(std::unordered_map<string, int> B, string name){
    
     std::cout << "started printing map  "<< name << std::endl; 
   
        for (auto i : B){
             std::cout << i.first << " : " << i.second << std::endl;
        }
       
    
    std::cout << "finished printing map"<< std::endl; 

}

void Print_CP(std::unordered_map<string, int>  C){
    
     
     std::cout << "       - - - -  Decreasing Values - - - -  "<< std::endl; 
   
        for (auto i:C){
            if(i.second<0){
             std::cout << i.first << " : " << (float(i.second)/float(window_size)) << std::endl;
            }
        }    
           std::cout << "       + + + + Increasing Values + + + + "<< std::endl; 
   
        for (auto i:C){
            if(i.second>0){
             std::cout << i.first << " : " << (float(i.second)/float(window_size)) << std::endl;
            }
        }  

}

// print a queue of unordered maps
void Print_queue(std::queue<std::unordered_map<string, int>> Q, string name){
     std::unordered_map<string, int> m;
     
     if (Q.empty()){
         std::cout << "Queue "<< name << " is empty"<<std::endl; 
     }else{
        std::cout << "started printing queue "<< name << " with size "<<Q.size()<< std::endl; 
     while (!Q.empty())
    {
        
        m= Q.front();
        for (auto i : m){
             std::cout << i.first << " : " << i.second << std::endl;
        }
        Q.pop();
        std::cout << "next"<< std::endl; 
    }
    std::cout << "finished printin queue"<< std::endl; 
     }

}


int Sum_itemset_count_in_Queue(std::queue<std::unordered_map<string, int>> Q, string itemset){
std::unordered_map<string, int> m;
     int sum=0;
     if (Q.empty()){
         std::cout << "Queue is empty"<<std::endl; 
     }else{
     while (!Q.empty())
    {
        
        m= Q.front();
        if (m.find(itemset) != m.end()){
           sum+=m[itemset];     
        }
        
        Q.pop();
       
    } 
    }
    return sum; 
}

// This void function scans the values in sl-counts (a new window slice ) and updates Hr and Hc  
// thresh is a threshold for detection of heavy sets and change points
// sl_count is a map of items to their counts maybe taken as slZ_count=Q0, slB_count=Qb, slL_count=Q2b,

void Update_Hev(std::unordered_map<string, int>  sl_counts){
   double cr;
   double cc;

   for(auto p: sl_counts){
        if(Br.find(p.first) != Br.end()){
            cr=Br[p.first];
        }else{
            cr=0;
        } 
        if(Bc.find(p.first) != Bc.end()){
            cc=Bc[p.first];
        }else{
            cc=0;
        } 
        
        
        if(cr>=thresh){
            Hr[p.first]=cr;
        }
        else{
            if (Hr.find(p.first) != Hr.end()){
               Hr.erase(p.first);
            }
        }
        if(cc>=thresh){
            Hc[p.first]=cc;
        }
        else{
            if (Hc.find(p.first) != Hc.end()){
                Hc.erase(p.first);
            }
        }
     
    }
    
}



// The function SINGScan receives as input a new set of transactions from the stream
// It updates the global variables: Qr, Qc and Br and Bc
// It also updates the heavy singletons in reference and current windows which will be passed to HScan

void SINGScan(std::vector<std::vector<string>> transactions) {
    // these maps store the counts of singletons: slZ_counts is Q0, slB_counts is Qb and slL_counts is Q2b 
    std::unordered_map<string, int> slZ_counts;
    std::unordered_map<string, int> slB_counts;
    std::unordered_map<string, int> slL_counts;
    for(auto tr : transactions) {
        for(auto i: tr){
          slZ_counts[i]++;
        }
    }

    // update Bc and Br 
    Qc.push(slZ_counts);
    for(auto p : slZ_counts){
        Bc[p.first]=Bc[p.first]+p.second;
    } 
   
  //  Print_map(slZ_counts," Q 0 ");
  //  Print_map(Bc, " Bc");
    
    slB_counts=Qc.front();
   
  //  Print_map(slB_counts, " Q b " );
  //  Print_map(Bc," Bc");
    Qc.pop();
    for(auto p : slB_counts){
        Bc[p.first]=Bc[p.first]-p.second;
        if( Bc[p.first]==0){
            Bc.erase(p.first);
        }
        Br[p.first]=Br[p.first]+p.second;
    } 

 
    Qr.push(slB_counts);
    slL_counts=Qr.front();
    Qr.pop();
    for(auto p : slL_counts){
        Br[p.first]=Br[p.first]-p.second;
        if(Br[p.first]==0){
            Br.erase(p.first);
        }
    } 

// Update sets of heavy singletons and change points 

   Update_Hev(slZ_counts);
   Update_Hev(slB_counts);
   Update_Hev(slL_counts);

   //Printing:
   //std::cout << "Printing Br and Bc"<<" @ slice: "<<clk/b<<std::endl;   
   //Print_map(Br," Br");
   //Print_map(Bc," Bc");
   //std::cout << "Printing Hr and Hc"<< std::endl; 
   //Print_map(Hr," Hr");
  // Print_map(Hc," Hc");


}
// this function updates the other queues which hold i-itemsets 
//  the itemsets are stored as strings seperated with commas 
void Items_Scan(std::vector<std::vector<string>> transactions) {

std::unordered_map<string, int> twocounts;
std::unordered_map<string, int> threecounts;
std::unordered_map<string, int> fourcounts;
std::unordered_map<string, int> fivecounts;
string str;
for(auto tr : transactions) {
        for(auto i: tr){
            for (auto j: tr){
             //   std::cout<<  "Pair"<< i<<","<<j<< std::endl; 
                if(i<j){
             //     std::cout<<  "Pair"<< i<<"<"<<j<< std::endl; 
                    str= i+","+j;
                    twocounts[str]++;       
                }// end of if of twocount
                if(scope>2){
                    for (auto k: tr){
                        if( i<j && j<k){
                            str= i+","+j+","+k;
                            threecounts[str]++;
                        }//end of if of three counts

                        if(scope>3){
                           for (auto l:tr){
                              if( i<j && j<k &&k<l){
                                   str= i+","+j+","+k+","+l;
                                   fourcounts[str]++;
                              }//end of if of fourcounts

                               if(scope>4){
                                    for (auto y:tr){
                                        if(i<j && j<k &&k<l && l<y){
                                           str= i+","+j+","+k+","+l+","+y;
                                           fivecounts[str]++;
                                        }//end of if of five counts
                                    }// end of four of fivecounts
                                }//end of scope: 5

                            }//end of for of 4 counts 
                        }// end of scope :4                 
                    }//end of for of 3 counts 
                }//end of scope 3
            }//end of for of 2 counts 
        }// end of first for
    }// end of four of reading transactions 

    
    
    //Print_map(twocounts, "two counts");
   
    if(!Q2c.empty() and !Q2r.empty()){
        Q2c.push(twocounts);
        Q2r.push(Q2c.front());

        Q2c.pop();
        Q2r.pop();
        
    }
    

   // Print_queue(Q2c, "2 itemsets, current");
   // Print_queue(Q2r, "2 itemsets, refrence");

    if(scope>2){
        if(!Q3c.empty() and !Q3r.empty()){
            Q3c.push(threecounts);
            Q3r.push(Q3c.front());
            Q3c.pop();
            Q3r.pop();
        }
    }


   // Print_queue(Q3c, "3 itemsets, current");
   // Print_queue(Q3r, "3 itemsets, reference");

    if(scope>3){
       // std::cout<<"Itemset scan  4 items"<<endl;
        if(!Q4c.empty() and !Q4r.empty()){
      
        Q4c.push(fourcounts);
        Q4r.push(Q4c.front());
        Q4c.pop();
        Q4r.pop();
        
        }
    }

   // Print_queue(Q4c, "4 itemsets, current");
    //Print_queue(Q4r, "4 itemsets, reference");

    if(scope>4){
        //std::cout<<"Itemset scan 5 items"<<endl;
        if(!Q5c.empty() and !Q5r.empty()){
            Q5c.push(fivecounts);
            Q5r.push(Q5c.front());
            Q5c.pop();
            Q5r.pop();
        }
    }

   // Print_queue(Q5c, "5 itemsets, current");
   // Print_queue(Q5r, "5 itemsets, reference");
}

/// void function used to initialize the queue after a change 
void Scan_and_push_in_ref(std::vector<std::vector<string>> transactions) {

std::unordered_map<string, int> onecounts;
std::unordered_map<string, int> twocounts;
std::unordered_map<string, int> threecounts;
std::unordered_map<string, int> fourcounts;
std::unordered_map<string, int> fivecounts;
string str;
for(auto tr : transactions) {
        for(auto i: tr){
            onecounts[i]++; 
            Br[i]++;
            for (auto j: tr){
             //   std::cout<<  "Pair"<< i<<","<<j<< std::endl; 
                if(i<j){
             //       std::cout<<  "Pair"<< i<<"<"<<j<< std::endl; 
                    str= i+","+j;
                    twocounts[str]++;       
                }
                if(scope>2){
                    for (auto k: tr){
                        if(i<j && j<k){
                           str= i+","+j+","+k;
                           threecounts[str]++;
                        }

                        if(scope>3){
                            for (auto l:tr){
                                if( i<j && j<k && k<l){
                                    str= i+","+j+","+k+","+l;
                                    fourcounts[str]++;
                                }//end of if of fourcounts

                                if(scope>4){
                                    for (auto y:tr){
                                        if(i<j && j<k && k<l&& l<y){
                                            str= i+","+j+","+k+","+l+","+y;
                                            fivecounts[str]++;
                                        }//end of if of five counts
                                    }//end of four of fivecounts
                                }//end of scope: 5

                            }//end of for of 4 counts 
                        }// end of scope :4
          
                    }//end of for for counting three items
                }//end of scope for item-sets larger than 2
            }
        }
    }
    Qr.push(onecounts);
    Q2r.push(twocounts);
    if(scope>2){
        Q3r.push(threecounts);
    }
    if(scope>3){
        Q4r.push(fourcounts);
    }
    if(scope>4){
        Q5r.push(fivecounts);
    }

}

void Scan_and_push_in_cur(std::vector<std::vector<string>> transactions) {

std::unordered_map<string, int> onecounts;
std::unordered_map<string, int> twocounts;
std::unordered_map<string, int> threecounts;
std::unordered_map<string, int> fourcounts;
std::unordered_map<string, int> fivecounts;
string str;

for(auto tr : transactions) {
        for(auto i: tr){
            onecounts[i]++; 
            Bc[i]++;
            for (auto j: tr){
             //   std::cout<<  "Pair"<< i<<","<<j<< std::endl; 
                if(i<j){
             //       std::cout<<  "Pair"<< i<<"<"<<j<< std::endl; 
                    str= i+","+j;
                    twocounts[str]++;       
                }
                if(scope>2){
                    for (auto k: tr){
                        if( i<j && j<k ){
                            str= i+","+j+","+k;
                            threecounts[str]++;
                        }

                        if(scope>3){
                            for (auto l:tr){
                                if(i<j && j<k && k<l){
                                    str= i+","+j+","+k+","+l;
                                    fourcounts[str]++;
                                }//end of if of fourcounts

                                if(scope>4){
                                    for (auto y:tr){
                                        if(i<j && j<k && k<l && l<y){
                                            str= i+","+j+","+k+","+l+","+y;
                                            fivecounts[str]++;
                                        }//end of if of five counts
                                    }// end of four of fivecounts
                                }//end of scope: 5
                            }//end of for of 4 counts 
                        }// end of scope :4
                    }//end of for
                }//end of scope    
            }
        }
    }
    Qc.push(onecounts);
    Q2c.push(twocounts);
    if(scope>2){
        Q3c.push(threecounts);
    }
  
    if(scope>3){
        Q4c.push(fourcounts);
    }
    if(scope>4){
        Q5c.push(fivecounts);
    }

}



std::unordered_map<string, int>  MergeHeavy(std::unordered_map<string, int> H_iminus1,std::unordered_map<string, int> H1, std::queue<std::unordered_map<string, int>> Q){
string candid;
int count_cand;
string seg;
std::unordered_map<string, int> Heavy;
for(auto p : H_iminus1){
    seg= p.first.substr(p.first.find_last_of(',')+1);
 //   cout<< "segment: "<< seg <<"."<<std::endl;
    for(auto s: H1){
     //   cout<< "p first, s first: "<< p.first<<" and "<< s.first <<std::endl;
       
        if(seg<s.first){
            candid=p.first+","+s.first;
      //      cout<< "candid:"<< candid <<std::endl;
        }
        

        count_cand= Sum_itemset_count_in_Queue(Q, candid);
        if (count_cand>= thresh){
            Heavy[candid]=count_cand;
        }

    }
}
//cout<< "heavy elements merged:"<<std::endl;
//Print_map(Heavy,"Heavy");
return Heavy; 

}


std::vector<string> itemset_to_vec(string transaction){
    std::size_t found = transaction.find(" ");
    string item;
    std::vector<string> transaction_vec={};

    while(found!=std::string::npos){
        item=transaction.substr(0,found);
       // std::cout<<item<<"."<<std::endl;
        transaction_vec.push_back(item);
        transaction= transaction.substr(found+1);
        found = transaction.find(" ");
    }

    return(transaction_vec);
}


void init(){

   
    string line;
    string str;
    std::vector<std::vector<string>> transaction_set;
    std::vector<string> transaction;
    
    for (int i=0; i<queue_size; i++){    
        transaction_set={};     
        for (int i=0; i<b; i++){            
            if(getline(newfile, line)){
                transaction=itemset_to_vec(line);                
                //Print_vec(transaction,"new transaction");
                transaction_set.push_back(transaction);
            }else{
                std::cout<<"End of stream"<<std::endl;
                end_of_stream=true;
                break;
              
            }
        }
        clk=clk+b;
        //std::cout << "queue item "<<std::endl; 
        Scan_and_push_in_ref(transaction_set);
    }  
  
    

    //std::cout << "end of ref "<<  std::endl; 

    for (int i=0; i<queue_size; i++){    
        transaction_set={};     
        for (int i=0; i<b; i++){            
            if(getline(newfile, line)){
                transaction=itemset_to_vec(line);                
                //Print_vec(transaction,"new transaction");
                transaction_set.push_back(transaction);
            }else{
               // std::cout<<"End of stream"<<std::endl;
                end_of_stream=true;
                break;
            }
        }
        clk=clk+b;
        //std::cout << "queue item "<<std::endl; 
        Scan_and_push_in_cur(transaction_set);
    }  

//std::cout << "end of cur "<< std::endl; 



std::cout << "Initialization finished"<< std::endl; 
/*
std::cout << "Printing Queues of singletons"<< std::endl; 
   Print_queue(Qr," Qr");
   Print_queue(Qc," Qc");

std::cout << "Printing Queues of 2-itemsets"<< std::endl; 
   Print_queue(Q2r," Q2r");
   Print_queue(Q2c," Q2c");

std::cout << "Printing Queues of 3-itemsets"<< std::endl; 
   Print_queue(Q3r," Q3r");
   Print_queue(Q3c," Q3c");

std::cout << "Printing Queues of 4-itemsets"<< std::endl; 
   Print_queue(Q4r," Q4r");
   Print_queue(Q4c," Q4c");

std::cout << "Printing Queues of 5-itemsets"<< std::endl; 
   Print_queue(Q5r," Q5r");
   Print_queue(Q5c," Q5c");


std::cout << "Printing Br and Bc"<< std::endl; 
   Print_map(Br," Br");
   Print_map(Bc," Bc");

std::cout << "------------------------- "<< std::endl; 
std::cout << "-------------------------- "<< std::endl; 
*/
}


std::unordered_map<string, int> CheckHev(std::unordered_map<string, int>  C2,std::unordered_map<string, int> Heavc, std::unordered_map<string, int> Heavr,std::queue<std::unordered_map<string, int>> ref,std::queue<std::unordered_map<string, int>> cur){
    double cr;
    double cc;
    for(auto p: Heavr){
        cr=Heavr[p.first];
        if (Heavc.find(p.first)==Heavc.end()){
            cc=Sum_itemset_count_in_Queue(cur, p.first);  
        }else{
            cc=Heavc[p.first];
        }
       // std::cout<<p.first<<" current and refrence "<<cr<<","<<cc<<endl;
        if(cr-cc>= thresh || cc-cr>= thresh){  
         //   std::cout<<p.first<<" current and refrence "<<cr<<","<<cc<<" threshold "<<thresh<<endl; 
            C2[p.first]=cc-cr; 
        }
    }
    for(auto p: Heavc){
     
        cc=Heavc[p.first];
        if (Heavr.find(p.first)==Heavr.end()){
            cr=Sum_itemset_count_in_Queue(ref, p.first);  
        }else{
            cr=Heavr[p.first];
        }
       // std::cout<<p.first<<" current and refrence "<<cr<<","<<cc<<endl;
        if(cr-cc>= thresh || cc-cr>= thresh){    
          //  std::cout<<p.first<<" current and refrence "<<cr<<","<<cc<<" threshold "<<thresh<<endl; 
            C2[p.first]=cc-cr;
        }
    }
    return C2;

}

std::unordered_map<string, int> FindCh(){

    std::unordered_map<string, int> Heavc;
    std::unordered_map<string, int> Heavr;
    std::unordered_map<string, int>  C2 ={};
    Heavr=Hr;
    Heavc=Hc;
    double cr;
    double cc;
    for(auto p: Heavr){
        cr=Br[p.first];
        if (Bc.find(p.first)!=Bc.end()){
            cc=Bc[p.first];
        }else{
            cc=0;
        }
        if(cr-cc>= thresh || cc-cr>= thresh){  
          //  std::cout<<p.first<<" current and refrence"<<cr<<","<<cc<<"threshold"<<thresh<<endl; 
            C2[p.first]=cc-cr; 
        }
    }
    for(auto p: Heavc){
        cc=Bc[p.first];
        if (Br.find(p.first)!=Br.end()){
            cr=Br[p.first];
        }else{
            cr=0;
        }
        if(cr-cc>= thresh || cc-cr>= thresh){  
          //  std::cout<<p.first<<" current and refrence"<<cr<<","<<cc<<"threshold"<<thresh<<endl; 
            C2[p.first]=cc-cr; 
        }
    }   

    Heavr=MergeHeavy(Heavr,Hr,Q2r);
    Heavc=MergeHeavy(Heavc,Hc,Q2c);
    std::queue<std::unordered_map<string, int>> ref=Q2r;
    std::queue<std::unordered_map<string, int>> cur=Q2c;
    C2=CheckHev(C2, Heavc,  Heavr, ref,cur);
 
    if(scope>2){
        Heavr=MergeHeavy(Heavr,Hr,Q3r);
        Heavc=MergeHeavy(Heavc,Hc,Q3c);
        //std::cout<<"** 3 itemset heavy**"<<endl;
        //Print_map(Heavr,"3 itemsets heavy refrence");
        //Print_map(Heavc,"3 itemsets heavy current");
        ref=Q3r;
        cur=Q3c;
        C2=CheckHev(C2, Heavc,  Heavr, ref,cur);
    }

    if (scope>3){
        Heavr=MergeHeavy(Heavr,Hr,Q4r);
        Heavc=MergeHeavy(Heavc,Hc,Q4c);
        //std::cout<<"** 4 itemset heavy**"<<endl;
        //Print_map(Heavr,"4 itemsets heavy refrence");
        //Print_map(Heavc,"4 itemsets heavy current");
        ref=Q4r;
        cur=Q4c;
        C2=CheckHev(C2, Heavc,  Heavr, ref,cur);
    }
   // std::cout<<"printing after 4 itemsets processed"<<endl;
   // Print_map(C2,"changed points");
    if (scope>4){
        Heavr=MergeHeavy(Heavr,Hr,Q5r);
        Heavc=MergeHeavy(Heavc,Hc,Q5c);
        //std::cout<<"** 5 itemset heavy**"<<endl;
        //Print_map(Heavr,"5 itemsets heavy refrence");
        //Print_map(Heavc,"5 itemsets heavy current");
        ref=Q5r;
        cur=Q5c;
        C2=CheckHev(C2, Heavc,  Heavr, ref,cur);
    }


   // std::cout<<"before return"<<endl;
   // Print_map(C2,"changed points");
    return C2;  
}


void read_new_slice( ){
    string line;
  
    std::vector<std::vector<string>> transaction_set={};
    std::vector<string> transaction={};
    std::unordered_map<string, int>  C2 ={};
    std::unordered_map<string, int>  C1and2 ={};
    

       
        for (int i=0; i<b; i++){            
            if(getline(newfile, line)){
                transaction=itemset_to_vec(line);                
              //  Print_vec(transaction,"new transaction");
                transaction_set.push_back(transaction);
            }else{
                std::cout<<"End of stream"<<std::endl;
                end_of_stream=true;
                break;
                
            }
        }
    clk=clk+b;
    if(clk-prclk>5000){
        std::cout <<clk<<" transactions processed"<< std::endl; 
        prclk=clk;
    }
   // std::cout << "------------------------"<< std::endl; 
    SINGScan(transaction_set) ;
    Items_Scan(transaction_set);
    C2=FindCh();
    /*
    for (auto p : C2){
        if (C.find(p.first) != C.end()){
            C1and2[p.first]=p.second;
        }
    }
    */
    if(!C2.empty()){

        std::cout << "       ***************************************************************************** "<< std::endl; 
        std::cout << "***************** Printing changed points ";
        std::cout << "at time: "<<clk <<" , ";
        std::cout<< "slice: "<<clk/b<<" **********************"<<std::endl; 
        Print_CP(C2);
       
    }
    // uncomment this part for higher accuracy and larger delay
    /*
    if(!C1and2.empty()){
        std::cout << "****************** Printing changed points ";
        std::cout << "at time: "<<clk <<"******************"<<std::endl; 
        Print_map(C1and2," change points");
    }
    C=C2;
    */
    //std::cout << "------------------------"<< std::endl; 
      
}

int main(int argc, char *argv[]) {

    textfilename= argv[1];//"../streams/short_file";
    eps = atof(argv[2]);//0.1;
    eps_inv= 1.0/eps; //10;
    max_transaction_size= atoi(argv[3]); //8
    scope= atoi(argv[4]); //2; 
    delta_inv= atoi(argv[5]); //10;
    samples=((scope*log(max_transaction_size)+1)*log(eps_inv)+log(delta_inv))*pow(eps_inv,2);
    queue_size= atoi(argv[6]);//10;
    b=samples/queue_size;
    window_size= b*queue_size;
    g=atof(argv[7]);//0.35;
    thresh=window_size*(g-eps);

    std::cout<<"window size:"<< window_size<< endl;
    std::cout<<"b:"<< b<< endl;
    std::cout<<"threshold:"<< thresh<< endl;
    newfile.open(textfilename,ios::in); //open a file to perform read operation using file object
    if (newfile.is_open()){
        init();
        while(! end_of_stream){
              read_new_slice();        
        }
    }
    newfile.close();
    

    return 0;
}










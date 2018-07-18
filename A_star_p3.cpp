#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

//#include <ctime> // time_t
#include <chrono>  // for high_resolution_clock
#include <iomanip>
#include <omp.h>


using namespace std;
using namespace std::chrono;

int tam_map=500;

omp_lock_t lock;

double start_time; 
double finish_time;


high_resolution_clock::time_point tstart;
high_resolution_clock::time_point tfinish;


class point {
public:
    int x, y;

    point( int a = 0, int b = 0 )
    { 
      x = a; 
      y = b; 
    }

    bool operator ==( const point& o ) { return o.x == x && o.y == y; }
    point operator +( const point& o ) { return point( o.x + x, o.y + y ); }
   
};
 
class map {
public:
    char m[500][500];
    int w, h;

    map() {
        /*
        char t[8][8] = {
            {0, 0, 0, 0, 0, 0, 0, 0}, 
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 1, 1, 1, 0}, 
            {0, 0, 1, 0, 0, 0, 1, 0},
            {0, 0, 1, 0, 0, 0, 1, 0}, 
            {0, 0, 1, 1, 1, 1, 1, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0}
        };*/
      
      string line;
      ifstream myfile ("mapa.csv");

      w = h = tam_map;  
       //vector<vector<char>> mapa(100,vector<char>(100,0));
      int x=0;
      int y=0;
      int value=0;

      if (myfile.is_open())
      {
        while ( getline (myfile,line) )
        {
          if(x<=tam_map-1){

            y=0;
            for (int i = 0; i < line.size(); ++i)
            {
                if(line[i]=='1' || line[i]=='0' ){
                  //cout<<line[i]<<";";
                  //mapa[x][y] = line[i];

                  value = (int)line[i] - 48;
                  m[x][y] = value;
                  y++;  
                }
                
            }
            //cout<<x<<endl;
            //cout <<'\n';
            x++;
          }

        }
        myfile.close();
      }

      else cout << "Unable to open file";

     /* 
    for (int i = 0; i < 100; ++i)
     {
        for (int j = 0; j < 100; ++j)
        {
          cout<<m[i][j]<<",";
        }
        cout<<endl;
     }*/

       
        /*
        w = h = 100;
        for( int r = 0; r < h; r++ )
            for( int s = 0; s < w; s++ )
                m[s][r] = t[r][s];*/

    }

    int operator() ( int x, int y ) { return m[x][y]; }
    
    
};
 
class node {
public:
    point pos, parent;
    int dist, cost;
    
    bool operator == (const node& o ) { return pos == o.pos; }
    bool operator == (const point& o ) { return pos == o; }
    bool operator < (const node& o ) { return dist + cost < o.dist + o.cost; }
    
};
 
class aStar {
public:
    aStar() {
        neighbours[0] = point( -1, -1 );//<-v
        neighbours[1] = point(  1, -1 );//^<-
        neighbours[2] = point( -1,  1 );//<- ^ 
        neighbours[3] = point(  1,  1 );//-> ^
        
        neighbours[4] = point(  0, -1 );//v 
        neighbours[5] = point( -1,  0 );//<
        neighbours[6] = point(  0,  1 );//^ 
        neighbours[7] = point(  1,  0 );//>
    }
 
    int calcDist( point& p ){

        // distancia de Manhattan
        int x = end.x - p.x, y = end.y - p.y;
        return( x * x + y * y );
    }
 
    bool isValid( point& p ) {
        //verificamos que el punto este dentro del mapa
        
        //omp_set_lock(&lock);
        
        return ( p.x >-1 && p.y > -1 && p.x < m.w && p.y < m.h );
        
        //omp_unset_lock(&lock);
    }
 
    
    bool existPoint( point& p, int cost ) {
        //#pragma omp critical
        //{


        //omp_set_lock(&lock);

        std::list<node>::iterator i;
        i = std::find( closed.begin(), closed.end(), p );

        if( i != closed.end() ) {
            
            if( ( *i ).cost + ( *i ).dist < cost ){
                return true;
            } 
            else {

                //omp_set_lock(&lock);
                   closed.erase( i ); 
                //omp_unset_lock(&lock);
                 
                return false; 
            }
        }
        i = std::find( open.begin(), open.end(), p );
        if( i != open.end() ) {
            if( ( *i ).cost + ( *i ).dist < cost )
              { return true;

              }
            else 
              { 
                //omp_set_lock(&lock);
                   open.erase( i ); 
                //omp_unset_lock(&lock);
                
                return false; 
              }
        }
        return false;
        
        
        //omp_unset_lock(&lock);
        //}
    }
 
    bool fillOpen( node& n ) {
        
                    
        int stepCost, nc, dist;
        stepCost=1;
        point neighbour;
        bool rpta = false;

        //omp_set_dynamic(0);     // Explicitly disable dynamic teams
        //omp_set_num_threads(4); // Use 4 threads for all consecutive parallel regions

        //int x=0;
        
        neighbour = n.pos;
        //point actual = n.pos;
        int tid=0;
        node m1;

        node n2= n; 

        #pragma omp parallel num_threads(8)  //shared(rpta,n2) //private(actual,tid) 
        {



        #pragma omp for nowait //private(neighbour,m1,nc,dist) //schedule(static,1)
        for(int x = 0; x < 8; x++ ) {
            //cout<<"threads="<<omp_get_num_threads()<<endl;
            // one can make diagonals have different cost
            
            
            //omp_set_lock(&lock);
            

        #pragma omp critical
        {
            //stepCost = x < 4 ? 1 : 1;
      
            neighbour = n.pos + neighbours[x];
            //}
     
            //o.x + x; 
            //o.y + y;


         
            tid = omp_get_thread_num();
            cout<<"TID"<<tid<<endl;

            
            //omp_set_lock(&lock);
            //neighbour = actual.x + neighbours[tid].x;
            //neighbour = actual.y + neighbours[tid].y;
            //omp_unset_lock(&lock);
                
            //neighbour = neighbour + neighbours[x];

            cout<<"thread id "<< omp_get_thread_num()<<" vecino "<<x<<endl; 
            //cout<<"actual pos "<<actual.x<<" "<< actual.y <<endl;
            cout<<"vecino pos "<<neighbour.x<<" "<< neighbour.y <<endl;
            
          

        //#pragma omp critical{

            if( neighbour == end ){ 
                
                //return true;  
                rpta=true;
            }
        



        //#pragma omp critical
        //{    

            if( isValid( neighbour ) && m( neighbour.x, neighbour.y ) != 1 ) {
                nc = stepCost + n2.cost;
                dist = calcDist( neighbour );

                if( !existPoint( neighbour, nc + dist ) ) {
                    //node m;
                    
                    m1.cost = nc; m1.dist = dist;
                    m1.pos = neighbour; 
                    m1.parent = n2.pos;

                    //omp_set_lock(&lock);

                    open.push_back( m1 );

                    //omp_unset_lock(&lock);
                }
            }
            
        }            
            //omp_unset_lock(&lock);

            //#pragma omp critical{}

        }

        }

         cout<<"FINISH neighbours "<<endl<<endl;
        //return false;
         return rpta;

        /* 
        if(rpta==true){
            return true;
        }else{
            return false;
        }*/

    }
 
 
    int path( std::list<point>& path ) {


        path.push_front( end );
        int cost = 1 + closed.back().cost; 
        path.push_front( closed.back().pos );
        point parent = closed.back().parent;
 
        for( std::list<node>::reverse_iterator i = closed.rbegin(); i != closed.rend(); i++ ) {
            if( ( *i ).pos == parent && !( ( *i ).pos == start ) ) {
                path.push_front( ( *i ).pos );
                parent = ( *i ).parent;
            }
        }
        path.push_front( start );
        return cost;
    }

 
    map m; point end, start;
    point neighbours[8];
    std::list<node> open;
    std::list<node> closed;


    bool search( point& s, point& e, map& mp ) {
    node n; end = e; start = s; m = mp;

    n.cost = 0; n.pos = s; n.parent = 0; n.dist = calcDist( s );

    open.push_back( n );

    // Record start time
    //start = std::chrono::high_resolution_clock::now();

    omp_set_num_threads(4);
    
    //tstart = high_resolution_clock::now();
    start_time = omp_get_wtime();




    while( !open.empty() ) {
        //open.sort();
        //#pragma omp parallel  
        node n = open.front();
        open.pop_front();

 
        closed.push_back(n);
        if( fillOpen( n ) ){

            finish_time = omp_get_wtime();
            //tfinish = high_resolution_clock::now(); 
            return true;
        }
    }

    // Record end time
    //finish = std::chrono::high_resolution_clock::now();

    

    //elapsed = finish - start;

    //double tiempo = difftime (end,begin);


    return false;
    }

 
};
 
int main( int argc, char* argv[] ) {


    omp_init_lock(&lock);    

    
    map m;
    point s, e( tam_map-1, tam_map-1 );
    //point s, e(99,99);
    aStar as;
     
    if( as.search( s, e, m ) ) {

        std::list<point> path;
        int c = as.path( path );

        
        
        for( int y = -1; y <= tam_map; y++ ) {
            for( int x = -1; x <= tam_map; x++ ) {
                //x < 0 || y < 0 || x > 7 || y > 7 || 
                if( x < 0 || y < 0 || x > tam_map-1 || y > tam_map-1 ||  m( x, y ) == 1 ){
                    //std::cout << char(0xdb);
                    //cout << "\u25A0";
                    //cout << "\uxdb";
                    cout<<"\u2302";
                    //std::cout << (char)254u;
                }        
                else {
                    if( std::find( path.begin(), path.end(), point( x, y ) )!= path.end() )
                        std::cout << "x";
                    else std::cout << ".";
                }
            }
            std::cout << "\n";
        }
 
        std::cout << "\nPath cost " << c << ": ";
        for( std::list<point>::iterator i = path.begin(); i != path.end(); i++ ) {
            std::cout<< "(" << ( *i ).x << ", " << ( *i ).y << ") ";
        }
    
    }






    
    cout<<endl;



    
    
    //auto duration = duration_cast<microseconds>( tfinish - tstart ).count();

    //auto elapsed = ( tfinish - tstart ).count();
    //std::chrono::duration<double> elapsed = tfinish - tstart;

   //cout.setf(ios::fixed, ios::floatfield);
   //cout.setf(ios::showpoint);

    double elapsed = finish_time - start_time;
//std::cout << "Elapsed time: " <<std::setprecision(3) << elapsed.count() << " s\n";
    //std::cout << "Elapsed time: " <<std::setprecision(3) << elapsed.count() << " s\n";

std::cout << "Elapsed time: " <<std::setprecision(3) << elapsed << " s\n"; 

    //printf(" Elapsed time %.2f", elapsed.count());

    //cout<<"TIEMPO "<<tiempo<<endl;
    std::cout << "\n\n";
    return 0;
}

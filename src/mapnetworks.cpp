#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/connected_components.hpp>
#include "../include/correlations.h"
#include "../include/symbols.h"

using namespace std;

bool fifty_day_transitive(const FloatCrossCorrelation::CorrelationsRef corr)
{
    // static const float sqrt_half = sqrt(0.5);
    
    return (0.975 <= corr.fifty_day);
}


template< class MeetsCriteria >
void make_clusters_for_date(const DateIndex::IndexType idate, MeetsCriteria& mc)
{
    using namespace boost;

    string filename = lexical_cast<string>(idate);
    WorkingDirectory current_dir(constants::lists_path.base_path());

    // Make sure there's something to do...
    if(!boost::filesystem::exists(filename))
    {
        cout << "Skipping day " << filename << " - no data." << endl;
        return;
    }

    cout << "Calculating clusters for day " << filename << "..." << endl
         << "   Loading symbols..." << endl;

    //  Load up a list of symbols. These are the vertices.
    SymbolVector vertex;
    load_symbols_from(vertex, filename.c_str());
    
    //  Load up the full set of correlations. This is all of the edges.
    current_dir.chdir(constants::correlations_path.base_path());

    cout << "   Loading cross correlations matrix ..." << endl;
    FloatCrossCorrelation unfiltered_edges;
    unfiltered_edges.load_from(filename.c_str());

    //  Construct graph from loaded set of vertices and some of the edges.
    typedef adjacency_matrix< undirectedS > Graph;
    Graph G(vertex.size());

    cout << "   Building graph ... " << endl;

    //  Filter for acceptable correlation values.
    FloatCrossCorrelation::Element edge;
    while( unfiltered_edges.get_next_element(edge) )
    {
        if( mc(unfiltered_edges.at(edge.index)) )
            add_edge(edge.rc.row, edge.rc.col, G);
    }
    cout << "   Total number of edges: " << num_edges(G) << endl;
    
    cout << "   Finding connected components ... " << endl;

    vector<int> component(num_vertices(G));

    cout << "   Vertex count=" << vertex.size() << "    Graph Vertex Count=" << component.size() << endl;
    int num = connected_components(G, &component[0]);

    cout << "Total number of components: " << num << endl;

    //  For each symbol, push back on end of deque[component[i]]
    //  Should add in symbols in alphabetical order...

    cout << "   Writing out clustering ..." << endl;

    deque< list< string > > cluster;
    cluster.resize(num);
    
    for (vector<int>::size_type i = 0; i < component.size(); ++i)
    {
        cluster[component[i]].push_back(vertex[i]);
    }

    //  Write components out to file as lines of text.
    current_dir.chdir(constants::lists_path.base_path());
    filename += ".clustering";
    {
        ofstream cfile(filename.c_str());
        
        BOOST_FOREACH( list< string > clust, cluster )
        {
            if ( 1 < clust.size() )
            {
                BOOST_FOREACH( string clu, clust )
                {
                    cfile << clu << " ";
                }
                cfile << endl << endl;
            }
        }
    }
}


int main(int , char* []) 
{
    //  Test with day 364 - lots of edges...
    //
    make_clusters_for_date(364, fifty_day_transitive);
    
    return 0;
}

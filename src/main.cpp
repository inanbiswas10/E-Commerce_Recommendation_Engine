#include "../include/Engine.hpp"
#include<iostream>
#include<chrono>
#include<iomanip>

using namespace std;
using namespace std::chrono;

int main () 
{
  Analytics_Engine engine;
  string path = "D:\\Computer Science (C++ Projects)\\ECommerce_Recommendation_Engine\\data\\user_logs.csv";

  cout<<"================================================================="<<endl;
  cout<<"   HIGH-PERFORMANCE HYBRID AI E-COMMERCE RECOMMENDATION ENGINE   "<<endl;
  cout<<"=================================================================" << endl;
  cout<<endl;

  cout<<"Starting data pipeline ingestion !!"<<endl;
  auto start = high_resolution_clock::now ();
    
  if (engine.load_csv (path)) 
  {
    auto end = high_resolution_clock::now ();
    duration <double> elapsed = (end-start);
        
    cout<<"Ingestion completed successfully !!"<<endl;
    cout<<"Time taken: "<<(elapsed.count ())<<" seconds.\n"<<endl;
        
    engine.print_summary ();
    engine.calculate_conversion_rate ();
    engine.find_most_popular_product ();

    engine.build_item_similarity_matrix ();

    engine.load_synthetic_embeddings (4);

    int sample_user = 11739;

    cout<<"\n[Hybrid AI Engine] Generating recommendations for User "<<(sample_user)<<" !!"<<endl;
    auto hybrid_start = high_resolution_clock::now ();
    vector <Recommendation> hybrid_predictions = engine.get_hybrid_recommendations (sample_user,0.6,5);
    auto hybrid_end = high_resolution_clock::now ();

    cout<<"Hybrid AI Engine computation took = "<<(duration <double> (hybrid_end-hybrid_start).count ())<<" seconds."<<endl;
    cout<<endl;
    cout<<"Top Recommended Product IDs (Hybrid Rank Fusion): "<<endl;
    cout<<endl;

    for (size_t value = 0;value < hybrid_predictions.size ();++value)
      cout<<(value + 1)<<". Product ID: "<<(hybrid_predictions [value].item_id)<<" (Hybrid Score: "<<(hybrid_predictions [value].score)<<")"<<endl;

    Evaluation_Metrics metrics = engine.evaluate_engine (5,500);

    cout<<"\n============================================================"<<endl;
    cout<<"           FINAL ENGINE PERFORMANCE BENCHMARK           "<<endl;
    cout<<"============================================================"<<endl;
    cout<<(fixed)<<(setprecision (4));
    cout<<" Precision at 5: "<<(metrics.precision_at_k*100.0)<<" %"<<endl;
    cout<<" Recall at 5: "<<(metrics.recall_at_k*100.0)<<" %"<<endl;
    cout<<"============================================================"<<endl;
  } 
  else 
    cout<<"Ingestion failed !!"<<endl;
  return 0;
}
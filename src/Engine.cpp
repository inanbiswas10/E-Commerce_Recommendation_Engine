#include "../include/Engine.hpp"
#include<iostream>
#include<fstream>
#include<sstream>
#include<chrono>
#include<cmath>
#include<algorithm>
#include<unordered_set>

using namespace std;

bool Analytics_Engine::load_csv (const string& file_path)
{
  ifstream file (file_path);
  if (!file.is_open ()) 
  {
    cerr<<"Error: Could not open file "<<(file_path)<<endl;
    return false;
  }
  string line;
  getline (file,line);

  while (getline (file,line)) 
  {
    stringstream ss (line);
    string user_id_str,item_id_str,action_type,timestamp_str;

    getline (ss,user_id_str,',');
    getline (ss,item_id_str,',');
    getline (ss,action_type,',');
    getline (ss,timestamp_str,',');

    int user_id = stoi (user_id_str);
    Action currentAction;
    currentAction.item_id = stoi (item_id_str);
    currentAction.action_type = action_type;
    currentAction.timestamp = stoll (timestamp_str);

    userMap [user_id].push_back (currentAction);
    itemToUsersMap [currentAction.item_id].push_back (user_id);
  }
  file.close ();
  return true;
}
void Analytics_Engine::print_summary () 
{
  cout<<"--- Ingestion Summary ---"<<endl;
  cout<<"Total unique users loaded: "<<(userMap.size ())<<endl;
    
  if (!userMap.empty ()) 
  {
    auto it = userMap.begin ();
    cout<<"Sample User ID: "<<(it->first)<<" performed "<<(it->second.size ())<<" action (s)."<<endl;
  }
}
void Analytics_Engine::calculate_conversion_rate ()
{
  long long total_actions = 0;
  long long total_purchases = 0;

  for (auto const& pair:userMap) 
  {
    const vector <Action>& actions = pair.second;
    total_actions = (total_actions + actions.size ());
    for (auto const& act:actions)
    {
      if (act.action_type == "purchase")
        total_purchases = (total_purchases + 1);
    }
  }
  double conversion_rate = ((double) (total_purchases)/total_actions)*100.0;
  cout<<"\n--- Behavioral Insights ---"<<endl;
  cout<<"Total Store Actions Processed: "<<(total_actions)<<endl;
  cout<<"Total Completed Purchases: "<<(total_purchases)<<endl;
  cout<<"Overall Platform Conversion Rate: "<<(conversion_rate)<<" %"<<endl;
}
void Analytics_Engine::find_most_popular_product ()
{
  unordered_map <int,int> product_views;
  for (auto const& pair:userMap)
  {
    const vector <Action>& actions = pair.second;
    for (auto const& act:actions)
    {
      if (act.action_type == "view" || act.action_type == "click")
        product_views [act.item_id] = (product_views [act.item_id] + 1);
    }
  }
  int most_popular_item = -1;
  int max_interactions = -1;
  for (auto const& pair:product_views)
  {
    int item_id = pair.first;
    int count = pair.second;
    if (count > max_interactions)
    {
      max_interactions = count;
      most_popular_item = item_id;
    }
  }
  cout<<"Most Popular Product ID: "<<(most_popular_item)<<" ("<<(max_interactions)<<" views/clicks)"<<endl;
}
double Analytics_Engine::calculate_cosine_similarity (const vector <Action>& userA,const vector<Action>& userB)
{
  unordered_map <int,double> weightsA,weightsB;
  for (auto const& act:userA)
  {
    double weight = (act.action_type == "purchase") ? 3.0:1.0;
    weightsA [act.item_id] = (weight + weightsA [act.item_id]);
  }
  for (auto const& act:userB)
  {
    double weight = (act.action_type == "purchase") ? 3.0:1.0;
    weightsB [act.item_id] = (weight + weightsB [act.item_id]);
  }
  double dot_product = 0.0;
  double normA = 0.0;
  double normB = 0.0;

  for (auto const& pair:weightsA)
  {
    int item_id = pair.first;
    double valA = pair.second;
    normA = ((valA*valA) + normA);
    if (weightsB.count (item_id))
      dot_product = (dot_product + (valA*weightsB [item_id]));
  }
  for (auto const& pair:weightsB) 
  {
    double valB = pair.second;
    normB = ((valB*valB)+normB);
  }
  if (normA == 0.0 || normB == 0.0) 
    return 0.0;
  return dot_product/(sqrt (normA)*sqrt (normB));
}
vector <Recommendation> Analytics_Engine::get_recommendations (int target_user_id,int topN) 
{
  vector <Recommendation> recs;
  if (!userMap.count (target_user_id)) 
  {
    cout<<"User ID "<<(target_user_id)<<" not found in system."<<endl;
    return recs;
  }
  const vector <Action>& target_user_actions = userMap [target_user_id];
  unordered_map <int,double> recommendation_scores;
  unordered_map <int,double> total_similarity;
    
  unordered_map <int,bool> already_interacted;
  for (auto const& act:target_user_actions) 
    already_interacted [act.item_id] = true;

  unordered_set <int> candidate_users;
  for (auto const& act:target_user_actions)
  {
    if (itemToUsersMap.count (act.item_id))
    {
      for (int candidate_id:itemToUsersMap [act.item_id])
      {
        if (candidate_id != target_user_id)
          candidate_users.insert (candidate_id);
      }
    }
  }
  for (int other_user_id:candidate_users)
  {
    const vector <Action>& other_user_actions = userMap [other_user_id];
    double similarity = calculate_cosine_similarity (target_user_actions,other_user_actions);
    if (similarity <= 0.0)
      continue;
    
    for (auto const& act:other_user_actions)
    {
      if (!already_interacted.count (act.item_id))
      {
        recommendation_scores [act.item_id] += (similarity*((act.action_type == "purchase") ? 3.0:1.0));
        total_similarity [act.item_id] += similarity;
      }
    }
  }
  for (auto const& pair:recommendation_scores) 
  {
    int item_id = pair.first;
    double final_score = pair.second/total_similarity [item_id];
    recs.push_back ({item_id,final_score});
  }
  if (recs.empty ())
  {
    unordered_map <int,int> product_views;
    for (auto const& pair:userMap)
    {
      for (auto const& act:pair.second)
      {
        if (!already_interacted.count (act.item_id) && (act.action_type == "view" || act.action_type == "click"))
          product_views [act.item_id] = (product_views [act.item_id] + 1);
      }
    }
    vector <pair <int,int>> popular_items (product_views.begin (),product_views.end ());
    sort (popular_items.begin (),popular_items.end (),[](const pair <int,int>& a,const pair<int,int>& b)
    {
      return (a.second > b.second);
    });
    for (int value = 0;value < min ((int) popular_items.size (),topN);++value)
      recs.push_back ({popular_items [value].first,(double)popular_items [value].second});
  }
  else
  {
    sort (recs.begin (),recs.end (),[] (const Recommendation& a,const Recommendation& b) 
    {
      return (a.score > b.score);
    });
  }
  if ((int) recs.size () > topN)
    recs.resize (topN);
  return recs;
}
void Analytics_Engine::build_item_similarity_matrix ()
{
  cout<<"\nBuilding Item-Item Similarity Matrix !!!"<<endl;
  auto start = chrono::high_resolution_clock::now ();

  unordered_map <int,double> itemNorms;
  for (auto const& pair:userMap)
  {
    for (auto const& act:pair.second)
    {
      double weight = (act.action_type == "purchase") ? 3.0 : 1.0;
      itemNorms [act.item_id] += (weight*weight);
    }
  }
  for (auto const& pair:userMap)
  {
    unordered_map <int,double> userItemWeights;
    for (auto const& act:pair.second)
    {
      double weight = (act.action_type == "purchase") ? 3.0 : 1.0;
      userItemWeights [act.item_id] += weight;
    }
    for (auto const& itemA_pair:userItemWeights)
    {
      for (auto const& itemB_pair:userItemWeights)
      {
        int itemA = itemA_pair.first;
        int itemB = itemB_pair.first;
        if (itemA >= itemB) 
          continue; 
        
        double dot = (itemA_pair.second*itemB_pair.second);
        itemSimilarityMatrix [itemA][itemB] += dot;
        itemSimilarityMatrix [itemB][itemA] += dot;
      }
    }
  }
  for (auto& itemA_pair:itemSimilarityMatrix)
  {
    int itemA = itemA_pair.first;
    for (auto& itemB_pair:itemA_pair.second)
    {
      int itemB = itemB_pair.first;
      double dot_product = itemB_pair.second;
      double normA = sqrt (itemNorms [itemA]);
      double normB = sqrt (itemNorms [itemB]);

      if (normA > 0 && normB > 0)
        itemSimilarityMatrix [itemA][itemB] = dot_product/(normA*normB);
      else
        itemSimilarityMatrix [itemA][itemB] = 0.0;
    }
  }
  auto end = chrono::high_resolution_clock::now ();
  chrono::duration <double> elapsed = (end-start);
  cout<<"Item-Item Matrix built successfully in "<<elapsed.count ()<<" seconds !!!"<<endl;
}
vector <Recommendation> Analytics_Engine::get_item_based_recommendations (int target_user_id,int topN,int exclude_holdout_id)
{
  vector <Recommendation> recs;
  if (!userMap.count (target_user_id)) 
    return recs;

  const vector <Action>& target_user_actions = userMap [target_user_id];
  unordered_map <int,bool> already_interacted;
  unordered_map <int,double> item_scores;

  for (auto const& act:target_user_actions)
  {
    if (act.item_id != exclude_holdout_id)
      already_interacted [act.item_id] = true;
  }
  for (auto const& act:target_user_actions)
  {
    if (act.item_id == exclude_holdout_id)
      continue;

    int user_item = act.item_id;
    double user_weight = (act.action_type == "purchase") ? 3.0 : 1.0;

    if (itemSimilarityMatrix.count (user_item))
    {
      for (auto const& sim_pair:itemSimilarityMatrix [user_item])
      {
        int similar_item = sim_pair.first;
        double sim_score = sim_pair.second;

        if (!already_interacted.count (similar_item))
          item_scores [similar_item] += (sim_score*user_weight);
      }
    }
  }
  for (auto const& pair:item_scores)
    recs.push_back ({pair.first,pair.second});

  sort (recs.begin (),recs.end (),[](const Recommendation& a,const Recommendation& b) 
  {
    return a.score > b.score;
  });
  if ((int) recs.size () > topN)
    recs.resize (topN);
  return recs;
}
double Analytics_Engine::calculate_vector_similarity (const vector <double>& vecA,const vector <double>& vecB)
{
  if (vecA.size () != vecB.size () || vecA.empty ()) 
    return 0.0;
  
  double dot_product = 0.0;
  double normA = 0.0;
  double normB = 0.0;

  for (size_t i = 0;i < vecA.size ();++i)
  {
    dot_product += (vecA [i]*vecB [i]);
    normA += (vecA [i]*vecA [i]);
    normB += (vecB [i]*vecB [i]);
  }
  if (normA == 0.0 || normB == 0.0) 
    return 0.0;
  return dot_product/(sqrt (normA)*sqrt (normB));
}
void Analytics_Engine::load_synthetic_embeddings (int vector_dim)
{
  cout<<"\n[AI Integration] Generating dense feature embeddings ("<<(vector_dim)<<"D vectors) !!!"<<endl;
  auto start = chrono::high_resolution_clock::now ();

  unordered_set <int> all_items;
  for (auto const& pair:userMap)
  {
    for (auto const& act:pair.second)
      all_items.insert (act.item_id);
  }
  for (int item_id : all_items)
  {
    vector <double> vec (vector_dim);
    for (int d = 0;d < vector_dim;++d)
    {
      double val = sin (item_id*0.13 + d*1.57);
      vec [d] = val;
    }
    productEmbeddings [item_id] = vec;
  }
  auto end = chrono::high_resolution_clock::now ();
  chrono::duration <double> elapsed = (end-start);
  cout<<"AI Vector Store initialized with "<<(productEmbeddings.size ())<<" product embeddings in "<<elapsed.count ()<<" seconds !!!"<<endl;
}
vector <Recommendation> Analytics_Engine::get_ai_semantic_recommendations (int target_item_id,int topN)
{
  vector <Recommendation> recs;
  if (!productEmbeddings.count (target_item_id)) 
    return recs;

  const vector <double>& target_vec = productEmbeddings [target_item_id];

  for (auto const& pair:productEmbeddings)
  {
    int candidate_item = pair.first;
    if (candidate_item == target_item_id) 
      continue;

    double similarity = calculate_vector_similarity (target_vec,pair.second);
    recs.push_back ({candidate_item,similarity});
  }
  sort (recs.begin (),recs.end (),[](const Recommendation& a,const Recommendation& b) 
  {
    return a.score > b.score;
  });
  if ((int) recs.size () > topN)
    recs.resize(topN);
  return recs;
}
vector <Recommendation> Analytics_Engine::get_hybrid_recommendations (int target_user_id,double alpha,int topN,int exclude_holdout_id)
{
  vector <Recommendation> hybrid_recs;
  if (!userMap.count (target_user_id)) 
    return hybrid_recs;

  vector <Recommendation> item_cf_recs = get_item_based_recommendations (target_user_id,20,exclude_holdout_id);

  if (item_cf_recs.empty ()) 
    return hybrid_recs;

  const auto& actions = userMap [target_user_id];
  int last_item = actions.back ().item_id;
  if (last_item == exclude_holdout_id && actions.size () >= 2)
    last_item = actions [actions.size ()-2].item_id;

  vector <Recommendation> ai_recs = get_ai_semantic_recommendations (last_item,50);

  unordered_map <int,double> ai_score_map;
  for (auto const& rec:ai_recs)
    ai_score_map [rec.item_id] = rec.score;

  double max_cf = item_cf_recs.front ().score;
  double min_cf = item_cf_recs.back ().score;

  unordered_map <int,double> hybrid_scores;

  for (auto const& rec:item_cf_recs)
  {
    double norm_cf = (max_cf > min_cf) ? (rec.score-min_cf)/(max_cf-min_cf) : 1.0;
    
    double raw_ai = ai_score_map.count (rec.item_id) ? ai_score_map [rec.item_id] : 0.0;
    double norm_ai = (raw_ai + 1.0)/2.0;

    double final_score = (alpha*norm_cf) + ((1.0-alpha)*norm_ai);
    hybrid_scores [rec.item_id] = final_score;
  }
  for (auto const& pair:hybrid_scores)
    hybrid_recs.push_back ({pair.first,pair.second});

  sort (hybrid_recs.begin (),hybrid_recs.end (),[](const Recommendation& a,const Recommendation& b) 
  {
    return a.score > b.score;
  });
  if ((int) hybrid_recs.size() > topN)
    hybrid_recs.resize (topN);
  return hybrid_recs;
}
Evaluation_Metrics Analytics_Engine::evaluate_engine (int k,int num_test_users)
{
  cout<<"\n[Evaluation Pipeline] Running Precision at "<<(k)<<" and Recall at "<<(k)<<" benchmark !!!"<<endl;
  auto start = chrono::high_resolution_clock::now ();

  double total_precision = 0.0;
  double total_recall = 0.0;
  int evaluated_count = 0;

  for (auto const& pair:userMap)
  {
    int user_id = pair.first;
    const auto& actions = pair.second;

    if (actions.size () < 3) 
      continue;

    int holdout_item = actions.back ().item_id;
    unordered_set <int> ground_truth = {holdout_item};

    vector <Recommendation> recs = get_hybrid_recommendations (user_id,0.6,k,holdout_item);

    int hits = 0;
    for (auto const& rec:recs)
    {
      if (ground_truth.count(rec.item_id))
        hits++;
    }
    double precision = (double) hits/k;
    double recall = (double) hits/ground_truth.size ();

    total_precision += precision;
    total_recall += recall;
    evaluated_count++;

    if (evaluated_count >= num_test_users) 
      break;
  }
  double avg_precision = (evaluated_count > 0) ? (total_precision/evaluated_count) : 0.0;
  double avg_recall = (evaluated_count > 0) ? (total_recall/evaluated_count) : 0.0;

  auto end = chrono::high_resolution_clock::now ();
  chrono::duration <double> elapsed = (end-start);
  cout<<"Evaluated "<<(evaluated_count)<<" test users in "<<(elapsed.count ())<<" seconds !!!"<<endl;

  return {avg_precision,avg_recall};
}


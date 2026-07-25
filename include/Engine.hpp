#ifndef ENGINE_HPP
#define ENGINE_HPP

#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>

struct Action
{
  std::string action_type;
  int item_id;
  long long timestamp;
};

struct Recommendation
{
  int item_id;
  double score;
};
struct Item_Similarity
{
  int item_id;
  double similarity_score;

};
struct Product_Embedding
{
  int item_id;
  std::vector <double> embedding_vector;
};
struct Evaluation_Metrics
{
  double precision_at_k;
  double recall_at_k;
};
class Analytics_Engine
{
  private:
    std::unordered_map <int,std::vector <Action>> userMap;
    std::unordered_map <int,std::vector <int>>itemToUsersMap;
    std::unordered_map <int,std::unordered_map <int,double>> itemSimilarityMatrix;
    std::unordered_map <int,std::vector <double>> productEmbeddings;

    double calculate_cosine_similarity (const std::vector <Action>& userA,const std::vector <Action>& userB);
    double calculate_vector_similarity (const std::vector <double>& vecA,const std::vector <double>& vecB);
  public:
    bool load_csv (const std::string& file_path);
    void print_summary ();
    void calculate_conversion_rate ();
    void find_most_popular_product ();
    void build_item_similarity_matrix ();
    void load_synthetic_embeddings (int vector_dim = 4);

    std::vector <Recommendation> get_ai_semantic_recommendations (int target_item_id,int topN = 5);
    std::vector <Recommendation> get_recommendations (int target_user_id,int topN = 5);
    std::vector <Recommendation> get_item_based_recommendations (int target_user_id,int topN = 5,int exclude_holdout_id = -1);
    std::vector <Recommendation> get_hybrid_recommendations (int target_user_id,double alpha = 0.6,int topN = 5,int exclude_holdout_id = -1);

    Evaluation_Metrics evaluate_engine (int k = 5,int num_test_users = 100);
};

#endif
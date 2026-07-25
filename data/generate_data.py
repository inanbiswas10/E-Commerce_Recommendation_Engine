import csv
import random

file_path = r"D:\Computer Science (C++ Projects)\ECommerce_Recommendation_Engine\data\user_logs.csv"

num_records = 1000000
actions = ["click","view","add_to_cart","purchase"]
print ("Generating 10,00,000 records...Please wait !!")

with open (file_path,mode = "w",newline = "") as file:
  writer = csv.writer (file)
  writer.writerow (["user_id","item_id","action_type","timestamp"])

  for value in range (num_records):
    user_id = random.randint (10000,99999)
    item_id = random.randint (100,5000)
    action_type = random.choice (actions)
    timestamp = 1770000000 + random.randint (0,500000)
    writer.writerow ([user_id,item_id,action_type,timestamp])

print (f"Dataset created successfully at: {file_path}")

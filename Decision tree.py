# -*- coding: utf-8 -*-
"""
Created on Sun Feb  8 19:23:40 2026

@author: Shreya
"""

import pandas as pd  
from sklearn.tree import DecisionTreeClassifier, export_text  
import matplotlib.pyplot as plt
from sklearn.tree import plot_tree
from sklearn.metrics import confusion_matrix, classification_report
import seaborn as sns


# -------------------------------------------------
# Dataset (Derived from Agronomic Ranges)
# -------------------------------------------------

data = {
    "Nitrogen": [
        115, 120, 125,      # Red Amaranthus
        90, 100, 110,       # Tomato
        95, 100, 105,       # Chilli
        95, 100, 105,       # Okra
        25, 30, 35          # Cowpea
    ],
    "Phosphorus": [
        45, 50, 55,         # Red Amaranthus
        55, 65, 75,         # Tomato
        55, 60, 65,         # Chilli
        65, 70, 75,         # Okra
        65, 70, 75          # Cowpea
    ],
    "Potassium": [
        130, 145, 155,     # Red Amaranthus
        190, 205, 215,     # Tomato
        165, 175, 185,     # Chilli
        120, 135, 145,     # Okra
        90, 100, 105       # Cowpea
    ],
    "pH": [
        6.1, 6.3, 6.4,     # Red Amaranthus
        5.9, 6.1, 6.2,     # Tomato
        6.1, 6.3, 6.4,     # Chilli
        6.3, 6.5, 6.7,     # Okra
        6.2, 6.4, 6.6      # Cowpea
    ],
    "Temperature": [
        28, 30, 32,        # Red Amaranthus
        22, 25, 27,        # Tomato
        24, 26, 28,        # Chilli
        27, 30, 32,        # Okra
        28, 30, 32         # Cowpea
    ],
    "EC": [
        1.3, 1.4, 1.5,     # Red Amaranthus
        2.3, 2.5, 2.7,     # Tomato
        1.9, 2.1, 2.3,     # Chilli
        1.7, 1.9, 2.1,     # Okra
        1.3, 1.5, 1.7      # Cowpea
    ],
    "Soil Moisture": [
        63, 65, 67,     # Red Amaranthus
        68, 70, 72,     # Tomato
        58, 60, 62,     # Chilli
        63, 65, 67,     # Okra
        53, 55, 57      # Cowpea
    ],
    "Crop": [
        "Red Amaranthus", "Red Amaranthus", "Red Amaranthus",
        "Tomato", "Tomato", "Tomato",
        "Chilli", "Chilli", "Chilli",
        "Okra", "Okra", "Okra",
        "Cowpea", "Cowpea", "Cowpea"
    ]
}

df = pd.DataFrame(data)

# -------------------------------------------------
#  Feature Selection
# -------------------------------------------------
X = df[["Nitrogen", "Phosphorus", "Potassium", "pH", "Temperature", "EC","Soil Moisture"]]
y = df["Crop"]

# -------------------------------------------------
# Train Decision Tree 
# -------------------------------------------------
model = DecisionTreeClassifier(
    criterion="gini",
    max_depth=4,             
    min_samples_leaf=2,       
    random_state=42          
)

model.fit(X, y)

# -------------------------------------------------
#  Extract Decision Tree Rules
# -------------------------------------------------
tree_rules = export_text(
    model,
    feature_names=list(X.columns)
)

print("DECISION TREE RULES:\n")
print(tree_rules)

# -------------------------------------------------
#  Test with Sample Input
# -------------------------------------------------
test_sample = pd.DataFrame([{
    "Nitrogen": 100,
    "Phosphorus": 70,
    "Potassium": 125,
    "pH": 6.3,
    "Temperature": 28,
    "EC": 1.8,
    "Soil Moisture" : 65
}])

prediction = model.predict(test_sample)
print("\nPredicted Crop:", prediction[0])

plt.figure(figsize=(22, 12))

plot_tree(
    model,
    feature_names=X.columns,
    class_names=model.classes_,
    filled=True,
    rounded=True,
    fontsize=10
)

plt.title("Decision Tree for Crop Recommendation", fontsize=16)
plt.show()

# -------------------------------------------------
#  Confusion Matrix & Heatmap
# -------------------------------------------------



y_pred = model.predict(X)


cm = confusion_matrix(y, y_pred, labels=model.classes_)


plt.figure(figsize=(8, 6))
sns.heatmap(
    cm,
    annot=True,
    fmt="d",
    cmap="YlGn",
    xticklabels=model.classes_,
    yticklabels=model.classes_,
    linewidths=0.5,
    linecolor="gray"
)
plt.title("Confusion Matrix – Crop Recommendation Model", fontsize=14)
plt.xlabel("Predicted Label", fontsize=12)
plt.ylabel("Actual Label", fontsize=12)
plt.xticks(rotation=30, ha="right")
plt.yticks(rotation=0)
plt.tight_layout()
plt.show()


print("\nCLASSIFICATION REPORT:\n")
print(classification_report(y, y_pred, target_names=model.classes_))
plt.figure(figsize=(8, 6))
plt.imshow(df[["Nitrogen","Phosphorus","Potassium","pH","Temperature","EC","Soil Moisture"]].corr(), 
           cmap='coolwarm', interpolation='nearest')
plt.colorbar()
plt.xticks(range(7), ["Nitrogen","Phosphorus","Potassium","pH","Temperature","EC","Soil Moisture"], rotation=45, ha='right')
plt.yticks(range(7), ["Nitrogen","Phosphorus","Potassium","pH","Temperature","EC","Soil Moisture"])
plt.title("Feature Correlation Heatmap")
plt.tight_layout()
plt.show()

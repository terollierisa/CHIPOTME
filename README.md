# CHIPOTME
ML Keywords Dataset
Overview
This repository contains the ML Keywords dataset, a collection of streams of keywords extracted from Machine Learning articles published on arXiv between April 2007 and March 2024.
Dataset Construction
The ML Keywords dataset was constructed from the arXiv dataset published by Cornell University, which consists of 1.7 million articles with relevant features such as article titles, authors, categories, abstracts, full text PDFs, and more. We focused on articles with the category cs.LG, which includes all articles published on all aspects of machine learning research. We extracted a total of 171,127 articles published between April 2007 and March 2024.
To convert the machine learning articles into a stream of itemsets, we followed these steps:
  - Keyword extraction: We converted each article into a set of keywords extracted using the open-source AI model Llama 3.
  - Frequent keyword filtering: We filtered out all keywords that appear in less than 1000 papers, ending with a total of 140 keywords.
  - Article filtering: We filtered out articles that contain fewer than two of these frequent keywords.

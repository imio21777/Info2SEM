# psql -U leap -d tpchdb -f impl/analysis_q5.sql | tee impl/analysis_q5_output1.txt

# psql -U leap -d tpchdb -f impl/optimize_q5.sql | tee impl/optimize_q5_output1.txt

# psql -U leap -d tpchdb -f impl/analysis_q9.sql | tee impl/analysis_q9_output.txt

# psql -U leap -d tpchdb -f impl/optimize_q9.sql | tee impl/optimize_q9_output.txt

# psql -U leap -d tpchdb -f impl/cleanup.sql | tee impl/cleanres.txt

# psql -U leap -d tpchdb -c "SHOW config_file;"

# psql -U leap -d tpchdb -f impl/analysis_q9.sql | tee impl/modifyparas_q9_output1.txt

# psql -U leap -d tpchdb -f impl/analysis_q9.sql | tee impl/modifyparas_q9_output2.txt

# psql -U leap -d tpchdb -f impl/analysis_q9.sql | tee impl/modifyparas_q9_output3.txt

# psql -U leap -d tpchdb -f impl/analysis_q5.sql | tee impl/modifyparas_q5_output1.txt
# psql -U leap -d tpchdb -f impl/analysis_q5.sql | tee impl/modifyparas_q5_output2.txt
# psql -U leap -d tpchdb -f impl/analysis_q5.sql | tee impl/modifyparas_q5_output3.txt

psql -U leap -d tpchdb -f impl/combined_q9.sql | tee impl/combined_q9_output1.txt
psql -U leap -d tpchdb -f impl/combined_q9.sql | tee impl/combined_q9_output2.txt
psql -U leap -d tpchdb -f impl/combined_q9.sql | tee impl/combined_q9_output3.txt
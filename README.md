# Jensen-Shannon Distance

*Completed in collaboration between Abhinav Sirohi (as3249) & Srinandini Marpaka (sm2237) for Systems Programming (01:198:214)*

## Testing
### Test Cases
- specifying no optional arguments (keeping default values)
- reading optional arguments
- resetting optional arguments in the middle of regular arguments
- combinations of all the different thread options
  - 1 directory thread
  - >1 directory threads
  - 1 file thread
  - >1 file threads
  - 1 analysis thread
  - >1 analysis threads
- combinations of the different types of regular arguments
  - regular files only
  - directories only (with and without subdirectories)
  - regular files and directories (with and without subdirectories)
  - multiple regular files and multiple directories
- files being joined to the parent directory name with a '/'
- calculating the JSD between 2 dissimilar files
- calculating the JSD between 2 similar files
- calculating the JSD between an empty and non-empty file
- less than 2 files
- output is sorted using qsort

### Strategy
#### Correctness
We ensured the correctness by making sure all of the combinations of files were printed at the end. We manually calculated the JSD for files that we inputted into the program and ensured the calculations matched.

#### Robustness
We tried files and directories of different sizes, word counts, and both long and short filenames.

All use of this code must be consistent with the [DCS Academic Integrity Policy](https://www.cs.rutgers.edu/academics/undergraduate/academic-integrity-policy/programming-assignments) and the [Rutgers University Code of Student Conduct](http://studentconduct.rutgers.edu/student-conduct-processes/university-code-of-student-conduct/).
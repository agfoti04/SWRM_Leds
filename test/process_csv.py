import sys

# Find the difference between each value in the dataset
def get_differences(dataset, columns):
    prev = dataset[0]
    transformation = [prev]

    for points in dataset[1:]:
        temp = points[:] # Copy to avoid direct borrowing
        for col in columns:
            points[col] = points[col]-prev[col]
        transformation.append(points)
        prev = temp

    return transformation

# Use the first argument as the filename
def csv_to_array():
    try:
        filename = sys.argv[1]
    except ValueError:
        print("Error: Missing arguments")

    try:
        csv_source = open(filename, "r")
        print(f"Reading from {filename}...")
    except OSError:
        print("Error: File does not exist")

    # Write the data to csv_raw
    line = csv_source.readline()
    csv_raw = []
    while line:
        csv_raw += [line.split(",")]
        line = csv_source.readline().strip()

    # Convert numbers to integers
    for i in range(len(csv_raw)):
        for j in range(len(csv_raw[i])):
            try:
                csv_raw[i][j] = int(csv_raw[i][j])
            except ValueError:
                csv_raw[i][j] = csv_raw[i][j]

    return csv_raw

# Begin main code
def main():
    dataset = csv_to_array()
    print(get_differences(dataset[1:], range(1, 2)))
    

if __name__ == "__main__":
    main()

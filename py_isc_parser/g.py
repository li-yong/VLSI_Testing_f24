import requests
import re
from collections import defaultdict

def fetch_google_doc_content(url):
    """
    Fetches the raw content from a Google Doc URL.
    Assumes the document is publicly accessible.
    """
    response = requests.get(url)
    response.raise_for_status()
    return response.text

def parse_grid_data(doc_content):
    """
    Parses the document content and extracts character and coordinates.
    """
    # Use regex to extract rows like '0 █ 0'
    pattern = r'(\d+)\s+([\u2580-\u259F\S])\s+(\d+)'
    matches = re.findall(pattern, doc_content)

    grid_data = []
    for match in matches:
        x, char, y = int(match[0]), match[1], int(match[2])
        grid_data.append((x, char, y))

    return grid_data

def build_and_print_grid(grid_data):
    """
    Builds and prints the grid based on the parsed data.
    """
    # Determine the size of the grid
    max_x = max(data[0] for data in grid_data)
    max_y = max(data[2] for data in grid_data)

    # Initialize a grid filled with spaces
    grid = [[' ' for _ in range(max_y + 1)] for _ in range(max_x + 1)]

    # Fill the grid with characters from the data
    for x, char, y in grid_data:
        grid[x][y] = char

    # Print the grid
    for row in grid:
        print(''.join(row))

def print_google_doc_grid(url):
    """
    Fetches, parses, and prints the grid of characters from the Google Doc.
    """
    # Step 1: Fetch the document content
    doc_content = fetch_google_doc_content(url)

    # Step 2: Parse the grid data
    grid_data = parse_grid_data(doc_content)

    # Step 3: Build and print the grid
    build_and_print_grid(grid_data)

# Example usage:
# Replace 'YOUR_GOOGLE_DOC_URL' with the actual Google Doc URL

G_DOC_URL = f'https://docs.google.com/document/d/e/2PACX-1vQGUck9HIFCyezsrBSnmENk5ieJuYwpt7YHYEzeNJkIb9OSDdx-ov2nRNReKQyey-cwJOoEKUhLmN9z/pub'

G_DOC_URL = f'https://docs.google.com/document/d/e/2PACX-1vRMx5YQlZNa3ra8dYYxmv-QIQ3YJe8tbI3kqcuC7lQiZm-CSEznKfN_HYNSpoXcZIV3Y_O3YoUB1ecq/pub'
print_google_doc_grid(G_DOC_URL)

import requests
from bs4 import BeautifulSoup
import pandas as pd

def decode_secret_message(url):
    response = requests.get(url)
    
    # Parse the HTML content 
    soup = BeautifulSoup(response.text, 'html.parser')
    
    # Find the table in the document (assuming it exists)
    table = soup.find('table')
    if not table:
        raise ValueError("No table found in the document")

    # Parse the table into a pandas DataFrame
    rows = []
    for tr in table.find_all('tr'):
        cells = [td.get_text(strip=True) for td in tr.find_all(['td', 'th'])]
        rows.append(cells)

    # Convert rows to a DataFrame
    df = pd.DataFrame(rows[1:], columns=rows[0])

    # Extract grid data from the DataFrame
    data = {}
    for _, row in df.iterrows():
        x = int(row['x-coordinate'])
        y = int(row['y-coordinate'])
        char = row['Character']
        data[(x, y)] = char

    max_x = max(x for x, y in data)
    max_y = max(y for x, y in data)

    grid = [[" " for _ in range(max_x + 1)] for _ in range(max_y + 1)]
    for (x, y), char in data.items():
        grid[y][x] = char

    for row in reversed(grid):
        print("".join(row))



G_DOC_URL = f'https://docs.google.com/document/d/e/2PACX-1vQGUck9HIFCyezsrBSnmENk5ieJuYwpt7YHYEzeNJkIb9OSDdx-ov2nRNReKQyey-cwJOoEKUhLmN9z/pub'
G_DOC_URL = f'https://docs.google.com/document/d/e/2PACX-1vRMx5YQlZNa3ra8dYYxmv-QIQ3YJe8tbI3kqcuC7lQiZm-CSEznKfN_HYNSpoXcZIV3Y_O3YoUB1ecq/pub'

decode_secret_message(G_DOC_URL)

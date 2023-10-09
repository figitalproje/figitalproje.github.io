import requests
from bs4 import BeautifulSoup
import csv

# Temel URL
BASE_URL = "https://www.sikayetvar.com/sahte?page="

# CSV dosyası için başlık bilgisi
headers = ['Marka', 'Şikayet Başlığı']

# Tüm veriyi saklamak için boş bir liste
data = []

# İlk 100 sayfa için döngü (bu sayıyı istediğiniz gibi değiştirebilirsiniz)
for page in range(1, 101):
    url = BASE_URL + str(page)
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')

    # Şikayet başlıklarını ve markalarını bulma
    complaints = soup.find_all('a', class_='complaint-layer')
    brands = soup.find_all('a', class_='link-reply')

    for complaint, brand in zip(complaints, brands):
        complaint_title = complaint.get('title')
        brand_name = brand.get('aria-label')
        data.append([brand_name, complaint_title])

# Veriyi CSV dosyasına yazma
with open('sikayetvar_data.csv', 'w', newline='', encoding='utf-8') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(headers)
    writer.writerows(data)

print("Veri çekme işlemi tamamlandı!")

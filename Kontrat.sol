// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "@openzeppelin/contracts/token/ERC721/extensions/ERC721URIStorage.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract SmartSeal is ERC721URIStorage, Ownable {
    enum SealStatus { Null, Open, Closed }
    mapping(uint256 => SealStatus) public sealStatus;
    mapping(uint256 => address) public publisher;
    mapping(uint256 => bool) public metadataModified;
    uint256 public price = 1000000000000000 wei; // Set initial price to 0.001 ETH
    uint256 public lastAssignedTokenId = 0;
    uint256 public lastMintedTokenId = 0;
    mapping(string => uint256) public barcodeToTokenId;
    mapping(uint256 => string) public tokenIdToBarcode;

    constructor() ERC721("SmartSeal", "SEAL") {}

    // Pre-mint NFTs
    function preMintSeal(string memory tokenURI, string memory barcode) public onlyOwner {
        uint256 newTokenId = lastMintedTokenId + 1;
        _mint(address(this), newTokenId);
        _setTokenURI(newTokenId, tokenURI);
        sealStatus[newTokenId] = SealStatus.Null; // Default status is Null
        lastMintedTokenId = newTokenId;
        // Barkodu saklıyoruz
        barcodeToTokenId[barcode] = newTokenId;
        tokenIdToBarcode[newTokenId] = barcode;
    }

    // Purchase and assign Publisher for multiple NFTs
    function purchaseSeal(uint256 amount) public payable {
        require(msg.value == price * amount, "Incorrect amount sent");

        for (uint256 i = 1; i <= amount; i++) {
            uint256 currentTokenId = lastAssignedTokenId + i;
            require(publisher[currentTokenId] == address(0), "Publisher already set for one of the tokens");

            _transfer(address(this), msg.sender, currentTokenId);
            publisher[currentTokenId] = msg.sender;
        }

        lastAssignedTokenId += amount;
    }

    // Publisher can modify metadata once
    function modifyMetadata(uint256 tokenId, string memory newURI) public {
        require(publisher[tokenId] == msg.sender, "Not the publisher");
        require(!metadataModified[tokenId], "Metadata already modified");
        _setTokenURI(tokenId, newURI);
        metadataModified[tokenId] = true;
    }
     function IDofBarcode(string memory barcode) public view returns (uint256) {
        return barcodeToTokenId[barcode];
    }
    // Set Seal Status
    function OpenSeal(uint256 tokenId) public {
       
            require(ownerOf(tokenId) == msg.sender, "Not the owner");
       
        sealStatus[tokenId] = SealStatus.Open;
    }

    function CloseSeal(uint256 tokenId, string memory newmetadata) public payable {
    
       require(msg.value==price,"Incorrect amount sent");
            require(publisher[tokenId] == msg.sender, "Not the publisher");
            require(ownerOf(tokenId) == msg.sender, "Sold");
       
        _setTokenURI(tokenId, newmetadata);
        sealStatus[tokenId] = SealStatus.Closed;
        
    }


    // Owner can modify the price
    function setPrice(uint256 newPrice) public onlyOwner {
        price = newPrice;
    }
}
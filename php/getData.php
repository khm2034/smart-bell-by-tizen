<?php 
    $con = mysqli_connect("localhost","root","sqlrootpw","db");
    if(!$con)
    {
        echo "MySQL 접속 에러 : ";
        echo mysqli_connect_error();
        exit();
    }
    mysqli_query("set names utf8");

    $data = file_get_contents('php://input');
    $arr = json_decode($data, TRUE);
    $pw = $arr['pw'];
    $statement = mysqli_prepare($con,"INSERT INTO test VALUES(?)");
    $bind = mysqli_stmt_bind_param($statement, "s", $pw);
    if($bind === false)
    {
        echo('바인드 실패 : '.mysqli_error($con));
        exit();
    }
    $exec = mysqli_stmt_execute($statement);
    if($bind === false)
    {
        echo('쿼리 실패 : '.mysqli_error($con));
        exit();
    }
    $response = array();
    $response["success"] = true;
  
    echo json_encode($response);
?>